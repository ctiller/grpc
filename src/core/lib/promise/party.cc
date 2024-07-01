// Copyright 2023 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/core/lib/promise/party.h"

#include <atomic>

#include "absl/base/thread_annotations.h"
#include "absl/log/check.h"
#include "absl/strings/str_format.h"

#include <grpc/support/log.h>
#include <grpc/support/port_platform.h>

#include "src/core/lib/event_engine/event_engine_context.h"
#include "src/core/lib/gprpp/sync.h"
#include "src/core/lib/iomgr/exec_ctx.h"
#include "src/core/lib/promise/activity.h"
#include "src/core/util/latent_see.h"

#ifdef GRPC_MAXIMIZE_THREADYNESS
#include "src/core/lib/gprpp/thd.h"       // IWYU pragma: keep
#include "src/core/lib/iomgr/exec_ctx.h"  // IWYU pragma: keep
#endif

namespace grpc_core {

///////////////////////////////////////////////////////////////////////////////
// PartySyncUsingAtomics

GRPC_MUST_USE_RESULT bool Party::RefIfNonZero() {
  auto count = state_.load(std::memory_order_relaxed);
  do {
    // If zero, we are done (without an increment). If not, we must do a CAS
    // to maintain the contract: do not increment the counter if it is already
    // zero
    if (count == 0) {
      return false;
    }
  } while (!state_.compare_exchange_weak(count, count + kOneRef,
                                         std::memory_order_acq_rel,
                                         std::memory_order_relaxed));
  LogStateChange("RefIfNonZero", count, count + kOneRef);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Party::Handle

// Weak handle to a Party.
// Handle can persist while Party goes away.
class Party::Handle final : public Wakeable {
 public:
  explicit Handle(Party* party) : party_(party) {}

  // Ref the Handle (not the activity).
  void Ref() { refs_.fetch_add(1, std::memory_order_relaxed); }

  // Activity is going away... drop its reference and sever the connection back.
  void DropActivity() ABSL_LOCKS_EXCLUDED(mu_) {
    mu_.Lock();
    CHECK_NE(party_, nullptr);
    party_ = nullptr;
    mu_.Unlock();
    Unref();
  }

  void WakeupGeneric(WakeupMask wakeup_mask,
                     void (Party::* wakeup_method)(WakeupMask))
      ABSL_LOCKS_EXCLUDED(mu_) {
    mu_.Lock();
    // Note that activity refcount can drop to zero, but we could win the lock
    // against DropActivity, so we need to only increase activities refcount if
    // it is non-zero.
    Party* party = party_;
    if (party != nullptr && party->RefIfNonZero()) {
      mu_.Unlock();
      // Activity still exists and we have a reference: wake it up, which will
      // drop the ref.
      (party->*wakeup_method)(wakeup_mask);
    } else {
      // Could not get the activity - it's either gone or going. No need to wake
      // it up!
      mu_.Unlock();
    }
    // Drop the ref to the handle (we have one ref = one wakeup semantics).
    Unref();
  }

  // Activity needs to wake up (if it still exists!) - wake it up, and drop the
  // ref that was kept for this handle.
  void Wakeup(WakeupMask wakeup_mask) override ABSL_LOCKS_EXCLUDED(mu_) {
    WakeupGeneric(wakeup_mask, &Party::Wakeup);
  }

  void WakeupAsync(WakeupMask wakeup_mask) override ABSL_LOCKS_EXCLUDED(mu_) {
    WakeupGeneric(wakeup_mask, &Party::WakeupAsync);
  }

  void Drop(WakeupMask) override { Unref(); }

  std::string ActivityDebugTag(WakeupMask) const override {
    MutexLock lock(&mu_);
    return party_ == nullptr ? "<unknown>" : party_->DebugTag();
  }

 private:
  // Unref the Handle (not the activity).
  void Unref() {
    if (1 == refs_.fetch_sub(1, std::memory_order_acq_rel)) {
      delete this;
    }
  }

  // Two initial refs: one for the waiter that caused instantiation, one for the
  // party.
  std::atomic<size_t> refs_{2};
  mutable Mutex mu_;
  Party* party_ ABSL_GUARDED_BY(mu_);
};

Wakeable* Party::Participant::MakeNonOwningWakeable(Party* party) {
  if (handle_ == nullptr) {
    handle_ = new Handle(party);
    return handle_;
  }
  handle_->Ref();
  return handle_;
}

Party::Participant::~Participant() {
  if (handle_ != nullptr) {
    handle_->DropActivity();
  }
}

Party::~Party() {}

void Party::CancelRemainingParticipants() {
  if ((state_.load(std::memory_order_relaxed) & kAllocatedMask) == 0) return;
  ScopedActivity activity(this);
  promise_detail::Context<Arena> arena_ctx(arena_.get());
  for (size_t i = 0; i < party_detail::kMaxParticipants; i++) {
    if (auto* p =
            participants_[i].exchange(nullptr, std::memory_order_acquire)) {
      p->Destroy();
    }
  }
}

std::string Party::ActivityDebugTag(WakeupMask wakeup_mask) const {
  return absl::StrFormat("%s [parts:%x]", DebugTag(), wakeup_mask);
}

Waker Party::MakeOwningWaker() {
  DCHECK(currently_polling_ != kNotPolling);
  IncrementRefCount();
  return Waker(this, 1u << currently_polling_);
}

Waker Party::MakeNonOwningWaker() {
  DCHECK(currently_polling_ != kNotPolling);
  return Waker(participants_[currently_polling_]
                   .load(std::memory_order_relaxed)
                   ->MakeNonOwningWakeable(this),
               1u << currently_polling_);
}

void Party::ForceImmediateRepoll(WakeupMask mask) {
  DCHECK(is_current());
  // Or in the bit for the currently polling participant.
  // Will be grabbed next round to force a repoll of this promise.
  const uint64_t prev_state = state_.fetch_or(mask, std::memory_order_relaxed);
  LogStateChange("ForceImmediateRepoll", prev_state, prev_state | mask);
}

void Party::RunLockedAndUnref(Party* party) {
  GRPC_LATENT_SEE_PARENT_SCOPE("Party::RunLocked");
#ifdef GRPC_MAXIMIZE_THREADYNESS
  Thread thd(
      "RunParty",
      [party]() {
        ApplicationCallbackExecCtx app_exec_ctx;
        ExecCtx exec_ctx;
        if (party->RunParty()) party->PartyIsOver();
      },
      nullptr, Thread::Options().set_joinable(false));
  thd.Start();
#else
  struct RunState;
  static thread_local RunState* g_run_state = nullptr;
  struct RunState {
    explicit RunState(Party* party) : running(party), next(nullptr) {}
    Party* running;
    Party* next;
    void Run() {
      g_run_state = this;
      do {
        GRPC_LATENT_SEE_INNER_SCOPE("run_one_party");
        running->RunPartyAndUnref();
        running = std::exchange(next, nullptr);
      } while (running != nullptr);
      DCHECK(g_run_state == this);
      g_run_state = nullptr;
    }
  };
  // If there is a party running, then we don't run it immediately
  // but instead add it to the end of the list of parties to run.
  // This enables a fairly straightforward batching of work from a
  // call to a transport (or back again).
  if (g_run_state != nullptr) {
    if (g_run_state->running == party || g_run_state->next == party) {
      // Already running or already queued.
      party->Unref();
      return;
    }
    if (g_run_state->next != nullptr) {
      // If there's already a different party queued, we're better off asking
      // event engine to run it so we can spread load.
      // We swap the oldest party to run on the event engine so that we don't
      // accidentally end up with a tail latency problem whereby one party
      // gets held for a really long time.
      std::swap(g_run_state->next, party);
      party->arena_->GetContext<grpc_event_engine::experimental::EventEngine>()
          ->Run([party]() {
            GRPC_LATENT_SEE_PARENT_SCOPE("Party::RunLocked offload");
            ApplicationCallbackExecCtx app_exec_ctx;
            ExecCtx exec_ctx;
            RunState{party}.Run();
          });
      return;
    }
    g_run_state->next = party;
    return;
  }
  RunState{party}.Run();
#endif
}

void Party::RunPartyAndUnref() {
  ScopedActivity activity(this);
  promise_detail::Context<Arena> arena_ctx(arena_.get());
  // Grab the current state, and clear the wakeup bits & add flag.
  uint64_t prev_state = state_.fetch_and(kRefMask | kLocked | kAllocatedMask,
                                         std::memory_order_acquire);
  LogStateChange("Run", prev_state,
                 prev_state & (kRefMask | kLocked | kAllocatedMask));
  DCHECK(prev_state & kLocked)
      << "Party should be locked; prev_state=" << prev_state;
  DCHECK_GE(prev_state & kRefMask, kOneRef);
  // From the previous state, extract which participants we're to wakeup.
  uint64_t wakeups = prev_state & kWakeupMask;
  // Now update prev_state to be what we want the CAS to see below.
  prev_state &= kRefMask | kLocked | kAllocatedMask;
  for (;;) {
    uint64_t keep_allocated_mask = kAllocatedMask;
    // For each wakeup bit...
    while (wakeups != 0) {
      uint64_t t = LowestOneBit(wakeups);
      const int i = CountTrailingZeros(t);
      wakeups ^= t;
      // If the bit is not set, skip.
      if (RunOneParticipant(i)) {
        const uint64_t allocated_bit = (1u << i << kAllocatedShift);
        keep_allocated_mask &= ~allocated_bit;
      }
    }
    // Try to CAS the state we expected to have (with no wakeups or adds)
    // back to unlocked (by masking in only the ref mask - sans locked bit).
    // If this succeeds then no wakeups were added, no adds were added, and we
    // have successfully unlocked.
    // Otherwise, we need to loop again.
    // Note that if an owning waker is created or the weak cas spuriously
    // fails we will also loop again, but in that case see no wakeups or adds
    // and so will get back here fairly quickly.
    // TODO(ctiller): consider mitigations for the accidental wakeup on owning
    // waker creation case -- I currently expect this will be more expensive
    // than this quick loop.
    if (state_.compare_exchange_weak(
            prev_state,
            (prev_state & (kRefMask | keep_allocated_mask)) - kOneRef,
            std::memory_order_acq_rel, std::memory_order_acquire)) {
      LogStateChange("Run:End", prev_state,
                     prev_state & (kRefMask | kAllocatedMask) - kOneRef);
      if ((prev_state & kRefMask) == kOneRef) {
        // We're done with the party.
        PartyIsOver();
      }
      return;
    }
    while (!state_.compare_exchange_weak(
        prev_state, prev_state & (kRefMask | kLocked | keep_allocated_mask))) {
      // Nothing to do here.
    }
    LogStateChange("Run:Continue", prev_state,
                   prev_state & (kRefMask | kLocked | keep_allocated_mask));
    DCHECK(prev_state & kLocked)
        << "Party should be locked; prev_state=" << prev_state;
    DCHECK_GE(prev_state & kRefMask, kOneRef);
    // From the previous state, extract which participants we're to wakeup.
    wakeups = prev_state & kWakeupMask;
    // Now update prev_state to be what we want the CAS to see once wakeups
    // complete next iteration.
    prev_state &= kRefMask | kLocked | keep_allocated_mask;
  }
}

bool Party::RunOneParticipant(int i) {
  GRPC_LATENT_SEE_INNER_SCOPE("Party::RunOneParticipant");
  // If the participant is null, skip.
  // This allows participants to complete whilst wakers still exist
  // somewhere.
  auto* participant = participants_[i].load(std::memory_order_acquire);
  if (participant == nullptr) {
    if (GRPC_TRACE_FLAG_ENABLED(promise_primitives)) {
      gpr_log(GPR_INFO, "%s[party] wakeup %d already complete",
              DebugTag().c_str(), i);
    }
    return false;
  }
  if (GRPC_TRACE_FLAG_ENABLED(promise_primitives)) {
    gpr_log(GPR_INFO, "%s begin job %d", DebugTag().c_str(), i);
  }
  // Poll the participant.
  currently_polling_ = i;
  bool done = participant->PollParticipantPromise();
  currently_polling_ = kNotPolling;
  if (done) {
    participants_[i].store(nullptr, std::memory_order_relaxed);
  }
  return done;
}

void Party::AddParticipants(Participant** participants, size_t count) {
  uint64_t state = state_.load(std::memory_order_acquire);
  uint64_t allocated;

  size_t slots[party_detail::kMaxParticipants];

  // Find slots for each new participant, ordering them from lowest available
  // slot upwards to ensure the same poll ordering as presentation ordering to
  // this function.
  WakeupMask wakeup_mask;
  do {
    wakeup_mask = 0;
    allocated = (state & kAllocatedMask) >> kAllocatedShift;
    for (size_t i = 0; i < count; i++) {
      auto new_mask = LowestOneBit(~allocated);
      wakeup_mask |= new_mask;
      allocated |= new_mask;
      slots[i] = CountTrailingZeros(new_mask);
    }
    // Try to allocate this slot and take a ref (atomically).
    // Ref needs to be taken because once we store the participant it could be
    // spuriously woken up and unref the party.
  } while (!state_.compare_exchange_weak(
      state, (state | (allocated << kAllocatedShift)) + kOneRef,
      std::memory_order_acq_rel, std::memory_order_acquire));
  LogStateChange("AddParticipantsAndRef", state,
                 (state | (allocated << kAllocatedShift)) + kOneRef);

  for (size_t i = 0; i < count; i++) {
    GRPC_TRACE_LOG(party_state, INFO)
        << "Party " << this << "                 AddParticipant: " << slots[i]
        << " " << participants[i];
    participants_[slots[i]].store(participants[i], std::memory_order_release);
  }

  // Now we need to wake up the party.
  state = state_.fetch_or(wakeup_mask | kLocked, std::memory_order_release);
  LogStateChange("AddParticipantsAndRef:Wakeup", state,
                 state | wakeup_mask | kLocked);

  // If the party was already locked, we're done; otherwise run the party.
  if ((state & kLocked) == 0) {
    RunLockedAndUnref(this);
  } else {
    Unref();
  }
}

void Party::Wakeup(WakeupMask wakeup_mask) {
  // Or in the wakeup bit for the participant, AND the locked bit.
  uint64_t prev_state = state_.fetch_or((wakeup_mask & kWakeupMask) | kLocked,
                                        std::memory_order_release);
  LogStateChange("ScheduleWakeup", prev_state,
                 prev_state | (wakeup_mask & kWakeupMask) | kLocked);
  // If the lock was not held now we hold it, so we need to run.
  if ((prev_state & kLocked) == 0) {
    RunLockedAndUnref(this);
  } else {
    Unref();
  }
}

void Party::WakeupAsync(WakeupMask wakeup_mask) {
  // Or in the wakeup bit for the participant, AND the locked bit.
  uint64_t prev_state = state_.fetch_or((wakeup_mask & kWakeupMask) | kLocked,
                                        std::memory_order_acq_rel);
  LogStateChange("ScheduleWakeup", prev_state,
                 prev_state | (wakeup_mask & kWakeupMask) | kLocked);
  if ((prev_state & kLocked) == 0) {
    arena_->GetContext<grpc_event_engine::experimental::EventEngine>()->Run(
        [this]() {
          ApplicationCallbackExecCtx app_exec_ctx;
          ExecCtx exec_ctx;
          RunLockedAndUnref(this);
        });
  } else {
    Unref();
  }
}

void Party::Drop(WakeupMask) { Unref(); }

void Party::PartyIsOver() {
  CancelRemainingParticipants();
  auto arena = std::move(arena_);
  this->~Party();
}

}  // namespace grpc_core
