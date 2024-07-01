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

#ifndef GRPC_SRC_CORE_LIB_PROMISE_PARTY_H
#define GRPC_SRC_CORE_LIB_PROMISE_PARTY_H

#include <stddef.h>
#include <stdint.h>

#include <atomic>
#include <string>
#include <utility>

#include "absl/base/attributes.h"
#include "absl/base/thread_annotations.h"
#include "absl/log/check.h"
#include "absl/strings/string_view.h"

#include <grpc/event_engine/event_engine.h>
#include <grpc/support/log.h>
#include <grpc/support/port_platform.h>

#include "src/core/lib/debug/trace.h"
#include "src/core/lib/gprpp/construct_destruct.h"
#include "src/core/lib/gprpp/crash.h"
#include "src/core/lib/gprpp/ref_counted.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/gprpp/sync.h"
#include "src/core/lib/promise/activity.h"
#include "src/core/lib/promise/context.h"
#include "src/core/lib/promise/detail/promise_factory.h"
#include "src/core/lib/promise/poll.h"
#include "src/core/lib/resource_quota/arena.h"
#include "src/core/util/useful.h"

namespace grpc_core {

namespace party_detail {

// Number of bits reserved for wakeups gives us the maximum number of
// participants.
static constexpr size_t kMaxParticipants = 16;

}  // namespace party_detail

// A Party is an Activity with multiple participant promises.
class Party : public Activity, private Wakeable {
 private:
  // Non-owning wakeup handle.
  class Handle;

  // One participant in the party.
  class Participant {
   public:
    // Poll the participant. Return true if complete.
    // Participant should take care of its own deallocation in this case.
    virtual bool PollParticipantPromise() = 0;

    // Destroy the participant before finishing.
    virtual void Destroy() = 0;

    // Return a Handle instance for this participant.
    Wakeable* MakeNonOwningWakeable(Party* party);

   protected:
    ~Participant();

   private:
    Handle* handle_ = nullptr;
  };

 public:
  Party(const Party&) = delete;
  Party& operator=(const Party&) = delete;

  static RefCountedPtr<Party> Make(RefCountedPtr<Arena> arena) {
    auto* arena_ptr = arena.get();
    return RefCountedPtr<Party>(arena_ptr->New<Party>(std::move(arena)));
  }

  // Spawn one promise into the party.
  // The promise will be polled until it is resolved, or until the party is shut
  // down.
  // The on_complete callback will be called with the result of the promise if
  // it completes.
  // A maximum of sixteen promises can be spawned onto a party.
  // promise_factory called to create the promise with the party lock taken;
  // after the promise is created the factory is destroyed.
  // This means that pointers or references to factory members will be
  // invalidated after the promise is created - so the promise should not retain
  // any of these.
  template <typename Factory, typename OnComplete>
  void Spawn(absl::string_view name, Factory promise_factory,
             OnComplete on_complete);

  template <typename Factory>
  auto SpawnWaitable(absl::string_view name, Factory factory);

  void Orphan() final { Crash("unused"); }

  // Activity implementation: not allowed to be overridden by derived types.
  void ForceImmediateRepoll(WakeupMask mask) final;
  WakeupMask CurrentParticipant() const final {
    DCHECK(currently_polling_ != kNotPolling);
    return 1u << currently_polling_;
  }
  Waker MakeOwningWaker() final;
  Waker MakeNonOwningWaker() final;
  std::string ActivityDebugTag(WakeupMask wakeup_mask) const final;

  void IncrementRefCount() {
    const uint64_t prev_state =
        state_.fetch_add(kOneRef, std::memory_order_relaxed);
    LogStateChange("IncrementRefCount", prev_state, prev_state + kOneRef);
  }
  void Unref() {
    uint64_t prev_state = state_.load(std::memory_order_relaxed);
    while (true) {
      uint64_t new_state = prev_state - kOneRef;
      if ((new_state & kRefMask) == 0) {
        new_state |= kLocked;
        if (state_.compare_exchange_weak(prev_state, new_state,
                                         std::memory_order_acq_rel)) {
          LogStateChange("Unref", prev_state, new_state);
          if ((prev_state & kLocked) == 0) PartyIsOver();
          return;
        }
      } else {
        if (state_.compare_exchange_weak(prev_state, new_state,
                                         std::memory_order_acq_rel)) {
          LogStateChange("Unref", prev_state, new_state);
          return;
        }
      }
    }
  }

  RefCountedPtr<Party> Ref() {
    IncrementRefCount();
    return RefCountedPtr<Party>(this);
  }

  Arena* arena() { return arena_.get(); }

  class BulkSpawner {
   public:
    explicit BulkSpawner(Party* party) : party_(party) {}
    ~BulkSpawner() {
      party_->AddParticipants(participants_, num_participants_);
    }

    template <typename Factory, typename OnComplete>
    void Spawn(absl::string_view name, Factory promise_factory,
               OnComplete on_complete);

   private:
    Party* const party_;
    size_t num_participants_ = 0;
    Participant* participants_[party_detail::kMaxParticipants];
  };

 protected:
  friend class Arena;

  // Derived types should be constructed upon `arena`.
  explicit Party(RefCountedPtr<Arena> arena) : arena_(std::move(arena)) {}
  ~Party() override;

  // Main run loop. Must be locked.
  // Polls participants and drains the add queue until there is no work left to
  // be done.
  void RunPartyAndUnref();

  bool RefIfNonZero();

 private:
  // Concrete implementation of a participant for some promise & oncomplete
  // type.
  template <typename SuppliedFactory, typename OnComplete>
  class ParticipantImpl final : public Participant {
    using Factory = promise_detail::OncePromiseFactory<void, SuppliedFactory>;
    using Promise = typename Factory::Promise;

   public:
    ParticipantImpl(absl::string_view name, SuppliedFactory promise_factory,
                    OnComplete on_complete)
        : on_complete_(std::move(on_complete)) {
      Construct(&factory_, std::move(promise_factory));
    }
    ~ParticipantImpl() {
      if (!started_) {
        Destruct(&factory_);
      } else {
        Destruct(&promise_);
      }
    }

    bool PollParticipantPromise() override {
      if (!started_) {
        auto p = factory_.Make();
        Destruct(&factory_);
        Construct(&promise_, std::move(p));
        started_ = true;
      }
      auto p = promise_();
      if (auto* r = p.value_if_ready()) {
        on_complete_(std::move(*r));
        delete this;
        return true;
      }
      return false;
    }

    void Destroy() override { delete this; }

   private:
    union {
      GPR_NO_UNIQUE_ADDRESS Factory factory_;
      GPR_NO_UNIQUE_ADDRESS Promise promise_;
    };
    GPR_NO_UNIQUE_ADDRESS OnComplete on_complete_;
    bool started_ = false;
  };

  template <typename SuppliedFactory>
  class PromiseParticipantImpl final
      : public RefCounted<PromiseParticipantImpl<SuppliedFactory>,
                          NonPolymorphicRefCount>,
        public Participant {
    using Factory = promise_detail::OncePromiseFactory<void, SuppliedFactory>;
    using Promise = typename Factory::Promise;
    using Result = typename Promise::Result;

   public:
    PromiseParticipantImpl(absl::string_view name,
                           SuppliedFactory promise_factory) {
      Construct(&factory_, std::move(promise_factory));
    }

    ~PromiseParticipantImpl() {
      switch (state_.load(std::memory_order_acquire)) {
        case State::kFactory:
          Destruct(&factory_);
          break;
        case State::kPromise:
          Destruct(&promise_);
          break;
        case State::kResult:
          Destruct(&result_);
          break;
      }
    }

    // Inside party poll: drive from factory -> promise -> result
    bool PollParticipantPromise() override {
      switch (state_.load(std::memory_order_relaxed)) {
        case State::kFactory: {
          auto p = factory_.Make();
          Destruct(&factory_);
          Construct(&promise_, std::move(p));
          state_.store(State::kPromise, std::memory_order_relaxed);
        }
          ABSL_FALLTHROUGH_INTENDED;
        case State::kPromise: {
          auto p = promise_();
          if (auto* r = p.value_if_ready()) {
            Destruct(&promise_);
            Construct(&result_, std::move(*r));
            state_.store(State::kResult, std::memory_order_release);
            waiter_.Wakeup();
            this->Unref();
            return true;
          }
          return false;
        }
        case State::kResult:
          Crash(
              "unreachable: promises should not be repolled after completion");
      }
    }

    // Outside party poll: check whether the spawning party has completed this
    // promise.
    Poll<Result> PollCompletion() {
      switch (state_.load(std::memory_order_acquire)) {
        case State::kFactory:
        case State::kPromise:
          return Pending{};
        case State::kResult:
          return std::move(result_);
      }
    }

    void Destroy() override { this->Unref(); }

   private:
    enum class State : uint8_t { kFactory, kPromise, kResult };
    union {
      GPR_NO_UNIQUE_ADDRESS Factory factory_;
      GPR_NO_UNIQUE_ADDRESS Promise promise_;
      GPR_NO_UNIQUE_ADDRESS Result result_;
    };
    Waker waiter_{GetContext<Activity>()->MakeOwningWaker()};
    std::atomic<State> state_{State::kFactory};
  };

  // State bits:
  // The atomic state_ field is composed of the following:
  //   - 24 bits for ref counts
  //     1 is owned by the party prior to Orphan()
  //     All others are owned by owning wakers
  //   - 1 bit to indicate whether the party is locked
  //     The first thread to set this owns the party until it is unlocked
  //     That thread will run the main loop until no further work needs to
  //     be done.
  //   - 1 bit to indicate whether there are participants waiting to be
  //   added
  //   - 16 bits, one per participant, indicating which participants have
  //   been
  //     woken up and should be polled next time the main loop runs.

  // clang-format off
  // Bits used to store 16 bits of wakeups
  static constexpr uint64_t kWakeupMask    = 0x0000'0000'0000'ffff;
  // Bits used to store 16 bits of allocated participant slots.
  static constexpr uint64_t kAllocatedMask = 0x0000'0000'ffff'0000;
  // Bit indicating locked or not
  static constexpr uint64_t kLocked        = 0x0000'0008'0000'0000;
  // Bits used to store 24 bits of ref counts
  static constexpr uint64_t kRefMask       = 0xffff'ff00'0000'0000;
  // clang-format on

  // Shift to get from a participant mask to an allocated mask.
  static constexpr size_t kAllocatedShift = 16;
  // How far to shift to get the refcount
  static constexpr size_t kRefShift = 40;
  // One ref count
  static constexpr uint64_t kOneRef = 1ull << kRefShift;

  // Destroy any remaining participants.
  // Needs to have normal context setup before calling.
  void CancelRemainingParticipants();

  // Run the locked part of the party until it is unlocked.
  static void RunLockedAndUnref(Party* party);
  // Called in response to Unref() hitting zero - ultimately calls PartyOver,
  // but needs to set some stuff up.
  // Here so it gets compiled out of line.
  void PartyIsOver();

  // Wakeable implementation
  GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION void Wakeup(
      WakeupMask wakeup_mask) final {
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

  void WakeupAsync(WakeupMask wakeup_mask) final;
  void Drop(WakeupMask wakeup_mask) final;

  // Add a participant (backs Spawn, after type erasure to ParticipantFactory).
  void AddParticipants(Participant** participant, size_t count);
  GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION void AddParticipant(
      Participant* participant) {
    uint64_t state = state_.load(std::memory_order_acquire);
    uint64_t allocated;
    size_t slot;

    // Find slots for each new participant, ordering them from lowest available
    // slot upwards to ensure the same poll ordering as presentation ordering to
    // this function.
    WakeupMask wakeup_mask;
    do {
      wakeup_mask = 0;
      allocated = (state & kAllocatedMask) >> kAllocatedShift;
      auto new_mask = LowestOneBit(~allocated);
      wakeup_mask |= new_mask;
      allocated |= new_mask;
      slot = CountTrailingZeros(new_mask);
      // Try to allocate this slot and take a ref (atomically).
      // Ref needs to be taken because once we store the participant it could be
      // spuriously woken up and unref the party.
    } while (!state_.compare_exchange_weak(
        state, (state | (allocated << kAllocatedShift)) + kOneRef,
        std::memory_order_acq_rel, std::memory_order_acquire));
    LogStateChange("AddParticipantsAndRef", state,
                   (state | (allocated << kAllocatedShift)) + kOneRef);

    GRPC_TRACE_LOG(party_state, INFO)
        << "Party " << this << "                 AddParticipant: " << slot
        << " [participant=" << participant << "]";
    participants_[slot].store(participant, std::memory_order_release);

    // Now we need to wake up the party.
    Wakeup(wakeup_mask);
  }
  bool RunOneParticipant(int i);

  GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION void LogStateChange(
      const char* op, uint64_t prev_state, uint64_t new_state,
      DebugLocation loc = {}) {
    if (GRPC_TRACE_FLAG_ENABLED(party_state)) {
      gpr_log(loc.file(), loc.line(), GPR_LOG_SEVERITY_INFO,
              "Party %p %30s: %016" PRIx64 " -> %016" PRIx64, this, op,
              prev_state, new_state);
    }
  }

  // Sentinal value for currently_polling_ when no participant is being polled.
  static constexpr uint8_t kNotPolling = 255;

  std::atomic<uint64_t> state_{kOneRef};
  uint8_t currently_polling_ = kNotPolling;
  // All current participants, using a tagged format.
  // If the lower bit is unset, then this is a Participant*.
  // If the lower bit is set, then this is a ParticipantFactory*.
  std::atomic<Participant*> participants_[party_detail::kMaxParticipants] = {};
  RefCountedPtr<Arena> arena_;
};

template <>
struct ContextSubclass<Party> {
  using Base = Activity;
};

template <typename Factory, typename OnComplete>
void Party::BulkSpawner::Spawn(absl::string_view name, Factory promise_factory,
                               OnComplete on_complete) {
  GRPC_TRACE_LOG(promise_primitives, INFO)
      << party_->DebugTag() << "[bulk_spawn] On " << this << " queue " << name
      << " (" << sizeof(ParticipantImpl<Factory, OnComplete>) << " bytes)";
  participants_[num_participants_++] = new ParticipantImpl<Factory, OnComplete>(
      name, std::move(promise_factory), std::move(on_complete));
}

template <typename Factory, typename OnComplete>
void Party::Spawn(absl::string_view name, Factory promise_factory,
                  OnComplete on_complete) {
  AddParticipant(new ParticipantImpl<Factory, OnComplete>(
      name, std::move(promise_factory), std::move(on_complete)));
}

template <typename Factory>
auto Party::SpawnWaitable(absl::string_view name, Factory promise_factory) {
  auto participant = MakeRefCounted<PromiseParticipantImpl<Factory>>(
      name, std::move(promise_factory));
  Participant* p = participant->Ref().release();
  AddParticipant(p);
  return [participant = std::move(participant)]() mutable {
    return participant->PollCompletion();
  };
}

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_PROMISE_PARTY_H
