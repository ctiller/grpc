/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <grpc/support/port_platform.h>

#include "src/core/lib/event_engine/iomgr_engine/timer.h"

#include <inttypes.h>

#include <atomic>
#include <limits>
#include <string>

#include "absl/strings/str_cat.h"

#include <grpc/support/alloc.h>
#include <grpc/support/cpu.h>
#include <grpc/support/log.h>
#include <grpc/support/sync.h>

#include "src/core/lib/debug/trace.h"
#include "src/core/lib/event_engine/iomgr_engine/timer_heap.h"
#include "src/core/lib/gpr/spinlock.h"
#include "src/core/lib/gpr/tls.h"
#include "src/core/lib/gpr/useful.h"
#include "src/core/lib/gprpp/time.h"

namespace grpc_event_engine {
namespace iomgr_engine {

grpc_core::Timestamp TimerList::Now() {
  return grpc_core::Timestamp::FromTimespecRoundDown(
      gpr_now(GPR_CLOCK_MONOTONIC));

  static const size_t kInvalidHeapIndex = std::numeric_limits<size_t>::max();
  static const double kAddDeadlineScale = 0.33;
  static const double kMinQueueWindowDuration = 0.01;
  static const double kMaxQueueWindowDuration = 1.0;
}  // namespace

static TimerCheckResult run_some_expired_timers(grpc_core::Timestamp now,
                                                grpc_core::Timestamp* next,
                                                grpc_error_handle error);

grpc_core::Timestamp TimerList::Shard::ComputeMinDeadline() {
  return heap.is_empty()
             ? queue_deadline_cap + grpc_core::Duration::Epsilon()
             : grpc_core::Timestamp::FromMillisecondsAfterProcessEpoch(
                   heap.Top()->deadline);
}

TimerList::Shard::Shard() : stats(1.0 / kAddDeadlineScale, 0.1, 0.5) {}

TimerList::TimerList() : min_timer_(Now()) {
  uint32_t i;

  shards_.resize(grpc_core::Clamp(2 * gpr_cpu_num_cores(), 1u, 32u));
  shard_queue_.resize(shards_.size());

  for (auto& shard : shards_) {
    shard.queue_deadline_cap = min_timer_;
    shard.shard_queue_index = i;
    shard.list.next = shard.list.prev = &shard.list;
    shard.min_deadline = shard.ComputeMinDeadline();
    shard_queue_[i] = &shard;
  }
}

namespace {
/* returns true if the first element in the list */
void ListJoin(Timer* head, Timer* timer) {
  timer->next = head;
  timer->prev = head->prev;
  timer->next->prev = timer->prev->next = timer;
}

void ListRemove(Timer* timer) {
  timer->next->prev = timer->prev;
  timer->prev->next = timer->next;
}
}  // namespace

void TimerList::SwapAdjacentShardsInQueue(uint32_t first_shard_queue_index) {
  Shard* temp;
  temp = shard_queue_[first_shard_queue_index];
  shard_queue_[first_shard_queue_index] =
      shard_queue_[first_shard_queue_index + 1];
  shard_queue_[first_shard_queue_index + 1] = temp;
  shard_queue_[first_shard_queue_index]->shard_queue_index =
      first_shard_queue_index;
  shard_queue_[first_shard_queue_index + 1]->shard_queue_index =
      first_shard_queue_index + 1;
}

void TimerList::NoteDeadlineChange(Shard* shard) {
  while (shard->shard_queue_index > 0 &&
         shard->min_deadline <
             shard_queue_[shard->shard_queue_index - 1]->min_deadline) {
    SwapAdjacentShardsInQueue(shard->shard_queue_index - 1);
  }
  while (shard->shard_queue_index < shards_.size() - 1 &&
         shard->min_deadline >
             shard_queue_[shard->shard_queue_index + 1]->min_deadline) {
    SwapAdjacentShardsInQueue(shard->shard_queue_index);
  }
}

void TimerList::TimerInit(Timer* timer, grpc_core::Timestamp deadline,
                          experimental::EventEngine::Closure* closure) {
  bool is_first_timer = false;
  Shard* shard = &shards_[grpc_core::HashPointer(timer, shards_.size())];
  timer->closure = closure;
  timer->deadline = deadline.milliseconds_after_process_epoch();

#ifndef NDEBUG
  timer->hash_table_next = nullptr;
#endif

  {
    grpc_core::MutexLock lock(&shard->mu);
    timer->pending = true;
    grpc_core::Timestamp now = grpc_core::ExecCtx::Get()->Now();
    if (deadline <= now) {
      deadline = now;
    }

    shard->stats.AddSample((deadline - now).millis() / 1000.0);

    if (deadline < shard->queue_deadline_cap) {
      is_first_timer = shard->heap.Add(timer);
    } else {
      timer->heap_index = kInvalidHeapIndex;
      ListJoin(&shard->list, timer);
    }
  }

  /* Deadline may have decreased, we need to adjust the main queue.  Note
     that there is a potential racy unlocked region here.  There could be a
     reordering of multiple TimerInit calls, at this point, but the < test
     below should ensure that we err on the side of caution.  There could
     also be a race with TimerCheck, which might beat us to the lock.  In
     that case, it is possible that the timer that we added will have already
     run by the time we hold the lock, but that too is a safe error.
     Finally, it's possible that the TimerCheck that intervened failed to
     trigger the new timer because the min_deadline hadn't yet been reduced.
     In that case, the timer will simply have to wait for the next
     TimerCheck. */
  if (is_first_timer) {
    grpc_core::MutexLock lock(&mu_);
    if (deadline < shard->min_deadline) {
      grpc_core::Timestamp old_min_deadline = shard_queue_[0]->min_deadline;
      shard->min_deadline = deadline;
      NoteDeadlineChange(shard);
      if (shard->shard_queue_index == 0 && deadline < old_min_deadline) {
#if GPR_ARCH_64
        min_timer_.store(deadline.milliseconds_after_process_epoch(),
                         std::memory_order_relaxed);
#else
        // On 32-bit systems, gpr_atm_no_barrier_store does not work on 64-bit
        // types (like grpc_core::Timestamp). So all reads and writes to
        // g_shared_mutables.min_timer varialbe under g_shared_mutables.mu
        g_shared_mutables.min_timer =
            deadline.milliseconds_after_process_epoch();
#endif
        Kick();
      }
    }
  }
}

bool TimerList::TimerCancel(Timer* timer) {
  Shard* shard = &shards_[grpc_core::HashPointer(timer, shards_.size())];
  grpc_core::MutexLock lock(&shard->mu);
  if (GRPC_TRACE_FLAG_ENABLED(grpc_timer_trace)) {
    gpr_log(GPR_INFO, "TIMER %p: CANCEL pending=%s", timer,
            timer->pending ? "true" : "false");
  }

  if (timer->pending) {
    timer->pending = false;
    if (timer->heap_index == kInvalidHeapIndex) {
      ListRemove(timer);
    } else {
      shard->heap.Remove(timer);
    }
    return true;
  }

  return false;
}

/* Rebalances the timer shard by computing a new 'queue_deadline_cap' and moving
   all relevant timers in shard->list (i.e timers with deadlines earlier than
   'queue_deadline_cap') into into shard->heap.
   Returns 'true' if shard->heap has at least ONE element
   REQUIRES: shard->mu locked */
bool TimerList::Shard::RefillHeap(grpc_core::Timestamp now) {
  /* Compute the new queue window width and bound by the limits: */
  double computed_deadline_delta = stats.UpdateAverage() * kAddDeadlineScale;
  double deadline_delta =
      grpc_core::Clamp(computed_deadline_delta, kMinQueueWindowDuration,
                       kMaxQueueWindowDuration);
  Timer *timer, *next;

  /* Compute the new cap and put all timers under it into the queue: */
  queue_deadline_cap = std::max(now, queue_deadline_cap) +
                       grpc_core::Duration::FromSecondsAsDouble(deadline_delta);

  for (timer = list.next; timer != &list; timer = next) {
    next = timer->next;
    auto timer_deadline =
        grpc_core::Timestamp::FromMillisecondsAfterProcessEpoch(
            timer->deadline);

    if (timer_deadline < shard->queue_deadline_cap) {
      if (GRPC_TRACE_FLAG_ENABLED(grpc_timer_check_trace)) {
        gpr_log(GPR_INFO, "  .. add timer with deadline %" PRId64 " to heap",
                timer_deadline.milliseconds_after_process_epoch());
      }
      ListRemove(timer);
      heap.Add(timer);
    }
  }
  return !heap.is_empty();
}

/* This pops the next non-cancelled timer with deadline <= now from the
   queue, or returns NULL if there isn't one.
   REQUIRES: shard->mu locked */
Timer* TimerList::Shard::PopOne(grpc_core::Timestamp now) {
  Timer* timer;
  for (;;) {
    if (heap.is_empty()) {
      if (now < queue_deadline_cap) return nullptr;
      if (!RefillHeap(now)) return nullptr;
    }
    timer = heap.Top();
    auto timer_deadline =
        grpc_core::Timestamp::FromMillisecondsAfterProcessEpoch(
            timer->deadline);
    if (timer_deadline > now) return nullptr;
    timer->pending = false;
    heap.Pop();
    return timer;
  }
}

/* REQUIRES: shard->mu unlocked */
std::vector<experimental::EventEngine::Closure*> TimerList::Shard::PopTimers(
    grpc_core::Timestamp now, grpc_core::Timestamp* new_min_deadline) {
  size_t n = 0;
  Timer* timer;
  gpr_mu_lock(&shard->mu);
  while ((timer = pop_one(shard, now))) {
    REMOVE_FROM_HASH_TABLE(timer);
    grpc_core::ExecCtx::Run(DEBUG_LOCATION, timer->closure,
                            GRPC_ERROR_REF(error));
    n++;
  }
  *new_min_deadline = compute_min_deadline(shard);
  gpr_mu_unlock(&shard->mu);
  if (GRPC_TRACE_FLAG_ENABLED(grpc_timer_check_trace)) {
    gpr_log(GPR_INFO, "  .. shard[%d] popped %" PRIdPTR,
            static_cast<int>(shard - g_shards), n);
  }
  return n;
}

std::vector<experimental::EventEngine::Closure*> TimerList::FindExpiredTimers(
    grpc_core::Timestamp now, grpc_core::Timestamp* next) {
#if GPR_ARCH_64
  // TODO(sreek): Using c-style cast here. static_cast<> gives an error (on
  // mac platforms complaining that gpr_atm* is (long *) while
  // (&g_shared_mutables.min_timer) is a (long long *). The cast should be
  // safe since we know that both are pointer types and 64-bit wide
  grpc_core::Timestamp min_timer =
      grpc_core::Timestamp::FromMillisecondsAfterProcessEpoch(
          gpr_atm_no_barrier_load((gpr_atm*)(&g_shared_mutables.min_timer)));
#else
  // On 32-bit systems, gpr_atm_no_barrier_load does not work on 64-bit types
  // (like grpc_core::Timestamp). So all reads and writes to
  // g_shared_mutables.min_timer are done under g_shared_mutables.mu
  gpr_mu_lock(&g_shared_mutables.mu);
  grpc_core::Timestamp min_timer = g_shared_mutables.min_timer;
  gpr_mu_unlock(&g_shared_mutables.mu);
#endif

  std::vector<experimental::EventEngine::Closure*> done;
  if (now < min_timer) {
    if (next != nullptr) *next = std::min(*next, min_timer);
    return done;
  }

  grpc_core::MutexLock lock(&mu_);

  while (shard_queue_[0]->min_deadline < now ||
         (now != grpc_core::Timestamp::InfFuture() &&
          shard_queue_[0]->min_deadline == now)) {
    grpc_core::Timestamp new_min_deadline;

    /* For efficiency, we pop as many available timers as we can from the
       shard.  This may violate perfect timer deadline ordering, but that
       shouldn't be a big deal because we don't make ordering guarantees. */
    shard_queue_[0]->PopTimers(now, &new_min_deadline, &done);

    /* An TimerInit() on the shard could intervene here, adding a new
       timer that is earlier than new_min_deadline.  However,
       TimerInit() will block on the mutex before it can call
       set_min_deadline, so this one will complete first and then the Addtimer
       will reduce the min_deadline (perhaps unnecessarily). */
    shard_queue_[0]->min_deadline = new_min_deadline;
    NoteDeadlineChange(shard_queue_[0]);
  }

  if (next) {
    *next = std::min(*next, shard_queue_[0]->min_deadline);
  }

#if GPR_ARCH_64
  // TODO(sreek): Using c-style cast here. static_cast<> gives an error (on
  // mac platforms complaining that gpr_atm* is (long *) while
  // (&g_shared_mutables.min_timer) is a (long long *). The cast should be
  // safe since we know that both are pointer types and 64-bit wide
  gpr_atm_no_barrier_store(
      (gpr_atm*)(&g_shared_mutables.min_timer),
      g_shard_queue[0]->min_deadline.milliseconds_after_process_epoch());
#else
  // On 32-bit systems, gpr_atm_no_barrier_store does not work on 64-bit
  // types (like grpc_core::Timestamp). So all reads and writes to
  // g_shared_mutables.min_timer are done under g_shared_mutables.mu
  g_shared_mutables.min_timer = g_shard_queue[0]->min_deadline;
#endif

  return result;
}

TimerCheckResult TimerList::TimerCheck(grpc_core::Timestamp* next) {
  // prelude
  grpc_core::Timestamp now = Now();

  /* fetch from a thread-local first: this avoids contention on a globally
     mutable cacheline in the common case */
  grpc_core::Timestamp min_timer =
      grpc_core::Timestamp::FromMillisecondsAfterProcessEpoch(
          min_timer_.load(std::memory_order_relaxed));

  if (now < min_timer) {
    if (next != nullptr) {
      *next = std::min(*next, min_timer);
    }
    return false;
  }

  if (!checker_mu_.TryLock()) return TimerCheckResult::kNotChecked;
  std::vector<experimental::EventEngine::Closure*> run =
      FindExpiredTimers(now, next);
  checker_mu_.Unlock();
  for (auto c : run) {
    c->Run();
  }
  return run.empty() ? TimerCheckResult::kCheckedAndEmpty
                     : TimerCheckResult::kFired;
}

}  // namespace iomgr_engine
}  // namespace grpc_event_engine
