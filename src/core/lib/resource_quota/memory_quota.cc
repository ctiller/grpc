// Copyright 2021 gRPC authors.
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

#include <grpc/support/port_platform.h>

#include "src/core/lib/resource_quota/memory_quota.h"

#include <thread>

#include "src/core/lib/promise/loop.h"
#include "src/core/lib/promise/race.h"
#include "src/core/lib/promise/seq.h"

namespace grpc_core {

//
// Reclaimer
//

ReclamationSweep::~ReclamationSweep() {
  if (memory_quota_ != nullptr) {
    memory_quota_->FinishReclamation(sweep_token_);
  }
}

//
// MemoryRequest
//

namespace {
size_t RoundUp(size_t size, size_t block_size) {
  return (size + block_size - 1) / block_size * block_size;
}
size_t RoundDown(size_t size, size_t block_size) {
  return size / block_size * block_size;
}
}  // namespace

MemoryRequest MemoryRequest::WithBlockSize(size_t block_size) const {
  MemoryRequest r(RoundUp(min_, block_size), RoundDown(max_, block_size));
  r.block_size_ = block_size;
  return r;
}

MemoryRequest MemoryRequest::Increase(size_t amount) const {
  MemoryRequest r(min_ + amount, max_ + amount);
  r.block_size_ = block_size_;
  return r;
}

//
// ReclaimerQueue
//

const ReclaimerQueue::Index ReclaimerQueue::kInvalidIndex;

ReclaimerQueue::Index ReclaimerQueue::Insert(
    RefCountedPtr<MemoryAllocator> allocator, ReclamationFunction reclaimer) {
  MutexLock lock(&mu_);
  Index index;
  if (free_entries_.empty()) {
    index = entries_.size();
    entries_.emplace_back(std::move(allocator), std::move(reclaimer));
  } else {
    index = free_entries_.back();
    free_entries_.pop_back();
    Entry& entry = entries_[index];
    entry.allocator = std::move(allocator);
    entry.reclaimer = std::move(reclaimer);
  }
  if (queue_.empty()) waker_.Wakeup();
  queue_.push(index);
  return index;
}

ReclamationFunction ReclaimerQueue::Cancel(Index index,
                                           MemoryAllocator* allocator) {
  MutexLock lock(&mu_);
  if (index >= entries_.size()) return nullptr;
  Entry& entry = entries_[index];
  if (entry.allocator.get() != allocator) return {};
  entry.allocator.reset();
  return std::move(entry.reclaimer);
}

Poll<ReclamationFunction> ReclaimerQueue::PollNext() {
  MutexLock lock(&mu_);
  while (true) {
    if (queue_.empty()) {
      waker_ = Activity::current()->MakeNonOwningWaker();
      return Pending{};
    }
    Index index = queue_.front();
    queue_.pop();
    free_entries_.push_back(index);
    Entry& entry = entries_[index];
    if (entry.allocator != nullptr) {
      entry.allocator.reset();
      return std::move(entry.reclaimer);
    }
  }
}

//
// MemoryAllocator
//

MemoryAllocator::MemoryAllocator(RefCountedPtr<MemoryQuota> memory_quota)
    : memory_quota_(memory_quota) {
  Reserve(sizeof(MemoryQuota));
}

MemoryAllocator::~MemoryAllocator() {
  Release(sizeof(MemoryQuota));
  GPR_ASSERT(free_bytes_.load(std::memory_order_acquire) == taken_bytes_);
  memory_quota_->Return(taken_bytes_);
}

void MemoryAllocator::Orphan() {
  {
    absl::MutexLock lock(&memory_quota_mu_);
    for (int i = 0; i < kNumReclamationPasses; i++) {
      memory_quota_->reclaimers_[i].Cancel(reclamation_indices_[i], this);
    }
  }
  InternallyRefCounted<MemoryAllocator>::Unref();
}

size_t MemoryAllocator::Reserve(MemoryRequest request) {
  while (true) {
    // Attempt to reserve memory from our pool.
    auto reservation = TryReserve(request);
    if (reservation.success) return reservation.size;
    // If that failed, grab more from the quota and retry.
    Replenish(reservation.size);
  }
}

MemoryAllocator::ReserveResult MemoryAllocator::TryReserve(
    MemoryRequest request) {
  // How much memory should we request? (see the scaling below)
  size_t scaled_request = request.max();
  // If we don't get what we want, how much should we request from the quota?
  // We actually ask for more than what we need to avoid needing to ask again.
  size_t take_request = 2 * request.max();
  // Scale the request down according to memory pressure if we have that
  // flexibility.
  if (request.min() != request.max()) {
    double pressure;
    {
      absl::MutexLock lock(&memory_quota_mu_);
      pressure = memory_quota_->InstantaneousPressure();
    }
    // Reduce allocation size proportional to the pressure > 80% usage.
    if (pressure > 0.8) {
      scaled_request =
          std::min(request.max(),
                   RoundUp(request.min() + (request.max() - request.min()) *
                                               (1.0 - pressure) / 0.2,
                           request.block_size()));
      take_request = request.min();
    }
  }

  // See how many bytes are available.
  size_t available = free_bytes_.load(std::memory_order_acquire);
  while (true) {
    // Does the current free pool satisfy the request?
    const size_t rounded_available = RoundDown(available, request.block_size());
    if (rounded_available < request.min()) {
      return {false, take_request - available};
    }
    // If so, grab as much as we need, up to what's available.
    const size_t reserve = std::min(rounded_available, scaled_request);
    // Try to reserve the requested amount.
    // If the amount of free memory changed through this loop, then available
    // will be set to the new value and we'll repeat.
    if (free_bytes_.compare_exchange_weak(available, available - reserve,
                                          std::memory_order_acq_rel,
                                          std::memory_order_release)) {
      return {true, reserve};
    }
  }
}

void MemoryAllocator::Replenish(size_t amount) {
  MutexLock lock(&memory_quota_mu_);
  // Take the requested amount from the quota.
  memory_quota_->Take(amount);
  // Record that we've taken it.
  taken_bytes_ += amount;
  // Add the taken amount to the free pool.
  free_bytes_.fetch_add(amount, std::memory_order_acq_rel);
  // See if we can add ourselves as a reclaimer.
  MaybeRegisterReclaimerLocked();
}

void MemoryAllocator::MaybeRegisterReclaimer() {
  MutexLock lock(&memory_quota_mu_);
  MaybeRegisterReclaimerLocked();
}

void MemoryAllocator::MaybeRegisterReclaimerLocked() {
  // If the reclaimer is already registered, then there's nothing to do.
  if (reclamation_indices_[0] != ReclaimerQueue::kInvalidIndex) return;
  // Grab references to the things we'll need
  auto self = Ref();
  auto memory_quota = memory_quota_;
  reclamation_indices_[0] = memory_quota_->reclaimers_[0].Insert(
      Ref(), [self, memory_quota](ReclamationSweep) {
        MutexLock lock(&self->memory_quota_mu_);
        // If the allocator's quota changed since this function was scheduled,
        // there's nothing more to do here.
        if (self->memory_quota_ != memory_quota) return;
        // Signal that we're no longer armed.
        self->reclamation_indices_[0] = ReclaimerQueue::kInvalidIndex;
        // Figure out how many bytes we can return to the quota.
        size_t return_bytes =
            self->free_bytes_.exchange(0, std::memory_order_acq_rel);
        // Subtract that from our outstanding balance.
        self->taken_bytes_ -= return_bytes;
        // And return them to the quota.
        self->memory_quota_->Return(return_bytes);
      });
}

void MemoryAllocator::Rebind(RefCountedPtr<MemoryQuota> memory_quota) {
  MutexLock lock(&memory_quota_mu_);
  if (memory_quota_ == memory_quota) return;
  // Return memory to the original memory quota.
  memory_quota_->Return(taken_bytes_);
  // Fetch back any reclaimers that are queued.
  ReclamationFunction reclaimers[kNumReclamationPasses];
  for (int i = 0; i < kNumReclamationPasses; i++) {
    reclaimers[i] = memory_quota_->reclaimers_[i].Cancel(
        absl::exchange(reclamation_indices_[i], ReclaimerQueue::kInvalidIndex),
        this);
  }
  // Switch to the new memory quota.
  memory_quota_ = std::move(memory_quota);
  // Drop our freed memory down to zero, to avoid needing to ask the new
  // quota for memory we're not currently using.
  taken_bytes_ -= free_bytes_.exchange(0, std::memory_order_acq_rel);
  // And let the new quota know how much we're already using.
  memory_quota_->Take(taken_bytes_);
  // Reinsert active reclaimers.
  for (int i = 0; i < kNumReclamationPasses; i++) {
    if (reclaimers[i] == nullptr) continue;
    reclamation_indices_[i] =
        memory_quota_->reclaimers_[i].Insert(Ref(), std::move(reclaimers[i]));
  }
}

void MemoryAllocator::PostReclaimer(ReclamationPass pass,
                                    ReclamationFunction fn) {
  MutexLock lock(&memory_quota_mu_);
  auto pass_num = static_cast<int>(pass);
  reclamation_indices_[pass_num] =
      memory_quota_->reclaimers_[pass_num].Insert(Ref(), std::move(fn));
}

//
// AtomicBarrier
//

Poll<AtomicBarrier::WaitPromise::Empty>
AtomicBarrier::WaitPromise::operator()() {
  if (barrier_->counter_.load(std::memory_order_acquire) != token_) {
    return Empty{};
  }
  barrier_->waker_ = Activity::current()->MakeNonOwningWaker();
  return Pending{};
}

uint64_t AtomicBarrier::NewToken() {
  return counter_.fetch_add(1, std::memory_order_relaxed);
}

void AtomicBarrier::Notify(uint64_t token) {
  if (counter_.compare_exchange_strong(token, token + 1,
                                       std::memory_order_acq_rel,
                                       std::memory_order_relaxed)) {
    waker_.Wakeup();
  }
}

//
// MemoryQuota
//

MemoryQuota::MemoryQuota() {
  auto self = WeakRef();

  auto reclamation_loop = Loop(Seq(
      [self]() -> Poll<int> {
        // If there's free memory we no longer need to reclaim memory!
        if (self->free_bytes_.load(std::memory_order_acquire) > 0) {
          return Pending{};
        }
        return 0;
      },
      [self]() {
        // Race biases to the first thing that completes... so this will
        // choose the highest priority/least destructive thing to do that's
        // available.
        return Race(self->reclaimers_[0].Next(), self->reclaimers_[1].Next(),
                    self->reclaimers_[2].Next(), self->reclaimers_[3].Next());
      },
      [self](ReclamationFunction reclaimer) {
        // One of the reclaimer queues gave us a way to get back memory.
        // Call the reclaimer with a token that contains enough to wake us
        // up again.
        uint64_t token = self->barrier_.NewToken();
        reclaimer(ReclamationSweep(self, token));
        // Return a promise that will wait for our barrier. This will be
        // awoken by the token above being destroyed. So, once that token is
        // destroyed, we'll be able to proceed.
        return self->barrier_.Wait(token);
      },
      []() -> LoopCtl<absl::Status> {
        // Continue the loop!
        return Continue{};
      }));

  reclaimer_activity_ = MakeActivity(
      std::move(reclamation_loop),
      [](std::function<void()> f) { std::thread(f).detach(); },
      [](absl::Status status) {
        GPR_ASSERT(status.code() == absl::StatusCode::kCancelled);
      });
}

void MemoryQuota::SetSize(size_t new_size) {
  size_t old_size = quota_size_.exchange(new_size, std::memory_order_relaxed);
  if (old_size < new_size) {
    // We're growing the quota.
    Return(new_size - old_size);
  } else {
    // We're shrinking the quota.
    Take(old_size - new_size);
  }
}

void MemoryQuota::Take(size_t amount) {
  // If there's a request for nothing, then do nothing!
  if (amount == 0) return;
  // Grab memory from the quota.
  auto prior = free_bytes_.fetch_sub(amount, std::memory_order_acq_rel);
  // If we push into overcommit, awake the reclaimer.
  if (prior >= 0 && prior < amount) {
    reclaimer_activity_->ForceWakeup();
  }
}

void MemoryQuota::Orphan() { reclaimer_activity_.reset(); }

void MemoryQuota::FinishReclamation(uint64_t token) { barrier_.Notify(token); }

void MemoryQuota::Return(size_t amount) {
  free_bytes_.fetch_add(amount, std::memory_order_relaxed);
}

}  // namespace grpc_core
