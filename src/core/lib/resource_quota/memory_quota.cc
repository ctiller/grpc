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

#include <inttypes.h>

#include <algorithm>
#include <atomic>
#include <tuple>
#include <type_traits>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/utility/utility.h"

#include "src/core/lib/debug/trace.h"
#include "src/core/lib/gpr/useful.h"
#include "src/core/lib/gprpp/global_config_env.h"
#include "src/core/lib/gprpp/mpscq.h"
#include "src/core/lib/promise/exec_ctx_wakeup_scheduler.h"
#include "src/core/lib/promise/loop.h"
#include "src/core/lib/promise/map.h"
#include "src/core/lib/promise/race.h"
#include "src/core/lib/promise/seq.h"
#include "src/core/lib/resource_quota/trace.h"

GPR_GLOBAL_CONFIG_DEFINE_BOOL(grpc_experimental_smooth_memory_presure, false,
                              "smooth the value of memory pressure over time");

namespace grpc_core {

// Maximum number of bytes an allocator will request from a quota in one step.
// Larger allocations than this will require multiple allocation requests.
static constexpr size_t kMaxReplenishBytes = 1024 * 1024;

// Minimum number of bytes an allocator will request from a quota in one step.
static constexpr size_t kMinReplenishBytes = 4096;

//
// Reclaimer
//

ReclamationSweep::~ReclamationSweep() {
  if (memory_quota_ != nullptr) {
    memory_quota_->FinishReclamation(sweep_token_, std::move(waker_));
  }
}

//
// ReclaimerQueue
//

struct ReclaimerQueue::QueuedNode
    : public MultiProducerSingleConsumerQueue::Node {
  explicit QueuedNode(RefCountedPtr<Handle> reclaimer_handle)
      : reclaimer_handle(std::move(reclaimer_handle)) {}
  RefCountedPtr<Handle> reclaimer_handle;
};

struct ReclaimerQueue::State {
  Mutex reader_mu;
  MultiProducerSingleConsumerQueue queue;  // reader_mu must be held to pop
  Waker waker ABSL_GUARDED_BY(reader_mu);

  ~State() {
    bool empty = false;
    do {
      delete static_cast<QueuedNode*>(queue.PopAndCheckEnd(&empty));
    } while (!empty);
  }
};

void ReclaimerQueue::Handle::Orphan() {
  if (auto* sweep = sweep_.exchange(nullptr, std::memory_order_acq_rel)) {
    sweep->RunAndDelete(absl::nullopt);
  }
  Unref();
}

void ReclaimerQueue::Handle::Run(ReclamationSweep reclamation_sweep) {
  if (auto* sweep = sweep_.exchange(nullptr, std::memory_order_acq_rel)) {
    sweep->RunAndDelete(std::move(reclamation_sweep));
  }
}

bool ReclaimerQueue::Handle::Requeue(ReclaimerQueue* new_queue) {
  if (sweep_.load(std::memory_order_relaxed)) {
    new_queue->Enqueue(Ref());
    return true;
  } else {
    return false;
  }
}

void ReclaimerQueue::Handle::Sweep::MarkCancelled() {
  // When we cancel a reclaimer we rotate the elements of the queue once -
  // taking one non-cancelled node from the start, and placing it on the end.
  // This ensures that we don't suffer from head of line blocking whereby a
  // non-cancelled reclaimer at the head of the queue, in the absence of memory
  // pressure, prevents the remainder of the queue from being cleaned up.
  MutexLock lock(&state_->reader_mu);
  while (true) {
    bool empty = false;
    std::unique_ptr<QueuedNode> node(
        static_cast<QueuedNode*>(state_->queue.PopAndCheckEnd(&empty)));
    if (node == nullptr) break;
    if (node->reclaimer_handle->sweep_.load(std::memory_order_relaxed) !=
        nullptr) {
      state_->queue.Push(node.release());
      break;
    }
  }
}

ReclaimerQueue::ReclaimerQueue() : state_(std::make_shared<State>()) {}

ReclaimerQueue::~ReclaimerQueue() = default;

void ReclaimerQueue::Enqueue(RefCountedPtr<Handle> handle) {
  if (state_->queue.Push(new QueuedNode(std::move(handle)))) {
    MutexLock lock(&state_->reader_mu);
    state_->waker.Wakeup();
  }
}

Poll<RefCountedPtr<ReclaimerQueue::Handle>> ReclaimerQueue::PollNext() {
  MutexLock lock(&state_->reader_mu);
  bool empty = false;
  // Try to pull from the queue.
  std::unique_ptr<QueuedNode> node(
      static_cast<QueuedNode*>(state_->queue.PopAndCheckEnd(&empty)));
  // If we get something, great.
  if (node != nullptr) return std::move(node->reclaimer_handle);
  if (!empty) {
    // If we don't, but the queue is probably not empty, schedule an immediate
    // repoll.
    Activity::current()->ForceImmediateRepoll();
  } else {
    // Otherwise, schedule a wakeup for whenever something is pushed.
    state_->waker = Activity::current()->MakeNonOwningWaker();
  }
  return Pending{};
}

//
// GrpcMemoryAllocatorImpl
//

GrpcMemoryAllocatorImpl::GrpcMemoryAllocatorImpl(
    std::shared_ptr<BasicMemoryQuota> memory_quota, std::string name)
    : memory_quota_(memory_quota), name_(std::move(name)) {
  memory_quota_->Take(taken_bytes_);
}

GrpcMemoryAllocatorImpl::~GrpcMemoryAllocatorImpl() {
  GPR_ASSERT(free_bytes_.load(std::memory_order_acquire) +
                 sizeof(GrpcMemoryAllocatorImpl) ==
             taken_bytes_.load(std::memory_order_relaxed));
  memory_quota_->Return(taken_bytes_);
}

void GrpcMemoryAllocatorImpl::Shutdown() {
  std::shared_ptr<BasicMemoryQuota> memory_quota;
  OrphanablePtr<ReclaimerQueue::Handle>
      reclamation_handles[kNumReclamationPasses];
  {
    MutexLock lock(&reclaimer_mu_);
    GPR_ASSERT(!shutdown_);
    shutdown_ = true;
    memory_quota = memory_quota_;
    for (size_t i = 0; i < kNumReclamationPasses; i++) {
      reclamation_handles[i] = absl::exchange(reclamation_handles_[i], nullptr);
    }
  }
}

size_t GrpcMemoryAllocatorImpl::Reserve(MemoryRequest request) {
  // Validate request - performed here so we don't bloat the generated code with
  // inlined asserts.
  GPR_ASSERT(request.min() <= request.max());
  GPR_ASSERT(request.max() <= MemoryRequest::max_allowed_size());
  while (true) {
    // Attempt to reserve memory from our pool.
    auto reservation = TryReserve(request);
    if (reservation.has_value()) {
      return *reservation;
    }
    // If that failed, grab more from the quota and retry.
    Replenish();
  }
}

absl::optional<size_t> GrpcMemoryAllocatorImpl::TryReserve(
    MemoryRequest request) {
  // How much memory should we request? (see the scaling below)
  size_t scaled_size_over_min = request.max() - request.min();
  // Scale the request down according to memory pressure if we have that
  // flexibility.
  if (scaled_size_over_min != 0) {
    const auto pressure_and_max_recommended_allocation_size =
        memory_quota_->InstantaneousPressureAndMaxRecommendedAllocationSize();
    double pressure = pressure_and_max_recommended_allocation_size.first;
    size_t max_recommended_allocation_size =
        pressure_and_max_recommended_allocation_size.second;
    // Reduce allocation size proportional to the pressure > 80% usage.
    if (pressure > 0.8) {
      scaled_size_over_min =
          std::min(scaled_size_over_min,
                   static_cast<size_t>((request.max() - request.min()) *
                                       (1.0 - pressure) / 0.2));
    }
    if (max_recommended_allocation_size < request.min()) {
      scaled_size_over_min = 0;
    } else if (request.min() + scaled_size_over_min >
               max_recommended_allocation_size) {
      scaled_size_over_min = max_recommended_allocation_size - request.min();
    }
  }

  // How much do we want to reserve?
  const size_t reserve = request.min() + scaled_size_over_min;
  // See how many bytes are available.
  size_t available = free_bytes_.load(std::memory_order_acquire);
  while (true) {
    // Does the current free pool satisfy the request?
    if (available < reserve) {
      return {};
    }
    // Try to reserve the requested amount.
    // If the amount of free memory changed through this loop, then available
    // will be set to the new value and we'll repeat.
    if (free_bytes_.compare_exchange_weak(available, available - reserve,
                                          std::memory_order_acq_rel,
                                          std::memory_order_acquire)) {
      return reserve;
    }
  }
}

void GrpcMemoryAllocatorImpl::MaybeDonateBack() {
  size_t free = free_bytes_.load(std::memory_order_relaxed);
  const size_t kReduceToSize = kMaxQuotaBufferSize / 2;
  while (true) {
    if (free <= kReduceToSize) return;
    size_t ret = free - kReduceToSize;
    if (free_bytes_.compare_exchange_weak(free, kReduceToSize,
                                          std::memory_order_acq_rel,
                                          std::memory_order_acquire)) {
      if (GRPC_TRACE_FLAG_ENABLED(grpc_resource_quota_trace)) {
        gpr_log(GPR_INFO, "[%p|%s] Early return %" PRIdPTR " bytes", this,
                name_.c_str(), ret);
      }
      GPR_ASSERT(taken_bytes_.fetch_sub(ret, std::memory_order_relaxed) >= ret);
      memory_quota_->Return(ret);
      return;
    }
  }
}

void GrpcMemoryAllocatorImpl::Replenish() {
  // Attempt a fairly low rate exponential growth request size, bounded between
  // some reasonable limits declared at top of file.
  auto amount = Clamp(taken_bytes_.load(std::memory_order_relaxed) / 3,
                      kMinReplenishBytes, kMaxReplenishBytes);
  // Take the requested amount from the quota.
  memory_quota_->Take(amount);
  // Record that we've taken it.
  taken_bytes_.fetch_add(amount, std::memory_order_relaxed);
  // Add the taken amount to the free pool.
  free_bytes_.fetch_add(amount, std::memory_order_acq_rel);
  // See if we can add ourselves as a reclaimer.
  MaybeRegisterReclaimer();
}

void GrpcMemoryAllocatorImpl::MaybeRegisterReclaimer() {
  // If the reclaimer is already registered, then there's nothing to do.
  if (registered_reclaimer_.exchange(true, std::memory_order_relaxed)) {
    return;
  }
  MutexLock lock(&reclaimer_mu_);
  if (shutdown_) return;
  // Grab references to the things we'll need
  auto self = shared_from_this();
  std::weak_ptr<EventEngineMemoryAllocatorImpl> self_weak{self};
  registered_reclaimer_ = true;
  InsertReclaimer(0, [self_weak](absl::optional<ReclamationSweep> sweep) {
    if (!sweep.has_value()) return;
    auto self = self_weak.lock();
    if (self == nullptr) return;
    auto* p = static_cast<GrpcMemoryAllocatorImpl*>(self.get());
    p->registered_reclaimer_.store(false, std::memory_order_relaxed);
    // Figure out how many bytes we can return to the quota.
    size_t return_bytes = p->free_bytes_.exchange(0, std::memory_order_acq_rel);
    if (return_bytes == 0) return;
    // Subtract that from our outstanding balance.
    p->taken_bytes_.fetch_sub(return_bytes);
    // And return them to the quota.
    p->memory_quota_->Return(return_bytes);
  });
}

//
// BasicMemoryQuota
//

class BasicMemoryQuota::WaitForSweepPromise {
 public:
  WaitForSweepPromise(std::shared_ptr<BasicMemoryQuota> memory_quota,
                      uint64_t token)
      : memory_quota_(std::move(memory_quota)), token_(token) {}

  struct Empty {};
  Poll<Empty> operator()() {
    if (memory_quota_->reclamation_counter_.load(std::memory_order_relaxed) !=
        token_) {
      return Empty{};
    } else {
      return Pending{};
    }
  }

 private:
  std::shared_ptr<BasicMemoryQuota> memory_quota_;
  uint64_t token_;
};

void BasicMemoryQuota::Start() {
  auto self = shared_from_this();

  // Reclamation loop:
  // basically, wait until we are in overcommit (free_bytes_ < 0), and then:
  // while (free_bytes_ < 0) reclaim_memory()
  // ... and repeat
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
        auto annotate = [](const char* name) {
          return [name](RefCountedPtr<ReclaimerQueue::Handle> f) {
            return std::make_tuple(name, std::move(f));
          };
        };
        return Race(Map(self->reclaimers_[0].Next(), annotate("compact")),
                    Map(self->reclaimers_[1].Next(), annotate("benign")),
                    Map(self->reclaimers_[2].Next(), annotate("idle")),
                    Map(self->reclaimers_[3].Next(), annotate("destructive")));
      },
      [self](
          std::tuple<const char*, RefCountedPtr<ReclaimerQueue::Handle>> arg) {
        auto reclaimer = std::move(std::get<1>(arg));
        if (GRPC_TRACE_FLAG_ENABLED(grpc_resource_quota_trace)) {
          double free = std::max(intptr_t(0), self->free_bytes_.load());
          size_t quota_size = self->quota_size_.load();
          gpr_log(GPR_INFO,
                  "RQ: %s perform %s reclamation. Available free bytes: %f, "
                  "total quota_size: %zu",
                  self->name_.c_str(), std::get<0>(arg), free, quota_size);
        }
        // One of the reclaimer queues gave us a way to get back memory.
        // Call the reclaimer with a token that contains enough to wake us
        // up again.
        const uint64_t token =
            self->reclamation_counter_.fetch_add(1, std::memory_order_relaxed) +
            1;
        reclaimer->Run(ReclamationSweep(
            self, token, Activity::current()->MakeNonOwningWaker()));
        // Return a promise that will wait for our barrier. This will be
        // awoken by the token above being destroyed. So, once that token is
        // destroyed, we'll be able to proceed.
        return WaitForSweepPromise(self, token);
      },
      []() -> LoopCtl<absl::Status> {
        // Continue the loop!
        return Continue{};
      }));

  reclaimer_activity_ =
      MakeActivity(std::move(reclamation_loop), ExecCtxWakeupScheduler(),
                   [](absl::Status status) {
                     GPR_ASSERT(status.code() == absl::StatusCode::kCancelled);
                   });
}

void BasicMemoryQuota::Stop() { reclaimer_activity_.reset(); }

void BasicMemoryQuota::SetSize(size_t new_size) {
  size_t old_size = quota_size_.exchange(new_size, std::memory_order_relaxed);
  if (old_size < new_size) {
    // We're growing the quota.
    Return(new_size - old_size);
  } else {
    // We're shrinking the quota.
    Take(old_size - new_size);
  }
}

void BasicMemoryQuota::Take(size_t amount) {
  // If there's a request for nothing, then do nothing!
  if (amount == 0) return;
  GPR_DEBUG_ASSERT(amount <= std::numeric_limits<intptr_t>::max());
  // Grab memory from the quota.
  auto prior = free_bytes_.fetch_sub(amount, std::memory_order_acq_rel);
  // If we push into overcommit, awake the reclaimer.
  if (prior >= 0 && prior < static_cast<intptr_t>(amount)) {
    if (reclaimer_activity_ != nullptr) reclaimer_activity_->ForceWakeup();
  }
}

void BasicMemoryQuota::FinishReclamation(uint64_t token, Waker waker) {
  uint64_t current = reclamation_counter_.load(std::memory_order_relaxed);
  if (current != token) return;
  if (reclamation_counter_.compare_exchange_strong(current, current + 1,
                                                   std::memory_order_relaxed,
                                                   std::memory_order_relaxed)) {
    if (GRPC_TRACE_FLAG_ENABLED(grpc_resource_quota_trace)) {
      double free = std::max(intptr_t(0), free_bytes_.load());
      size_t quota_size = quota_size_.load();
      gpr_log(GPR_INFO,
              "RQ: %s reclamation complete. Available free bytes: %f, "
              "total quota_size: %zu",
              name_.c_str(), free, quota_size);
    }
    waker.Wakeup();
  }
}

void BasicMemoryQuota::Return(size_t amount) {
  free_bytes_.fetch_add(amount, std::memory_order_relaxed);
}

std::pair<double, size_t>
BasicMemoryQuota::InstantaneousPressureAndMaxRecommendedAllocationSize() {
  static const bool kSmoothMemoryPressure =
      GPR_GLOBAL_CONFIG_GET(grpc_experimental_smooth_memory_presure);
  double free = free_bytes_.load();
  if (free < 0) free = 0;
  size_t quota_size = quota_size_.load();
  double size = quota_size;
  if (size < 1) return std::make_pair(1.0, 1);
  double pressure = (size - free) / size;
  if (pressure < 0.0) pressure = 0.0;
  if (pressure > 1.0) pressure = 1.0;
  if (kSmoothMemoryPressure) {
    pressure = pressure_tracker_.AddSampleAndGetEstimate(pressure);
  }
  return std::make_pair(pressure, quota_size / 16);
}

//
// PressureTracker
//

namespace memory_quota_detail {

double PressureTracker::AddSampleAndGetEstimate(double sample) {
  double max_so_far = max_this_round_.load(std::memory_order_relaxed);
  if (sample > max_so_far) {
    max_this_round_.compare_exchange_weak(max_so_far, sample,
                                          std::memory_order_relaxed,
                                          std::memory_order_relaxed);
  }
  // If memory pressure is almost done, immediately hit the brakes and report
  // full memory usage.
  if (sample >= 0.99) {
    report_.store(1.0, std::memory_order_relaxed);
  }
  update_.Tick([&](Duration dt) {
    // Reset the round tracker with the new sample.
    const double current_estimate =
        max_this_round_.exchange(sample, std::memory_order_relaxed);
    double report;
    if (current_estimate > 0.99) {
      // Under very high memory pressure we... just max things out.
      report = pid_.Update(1e99, 1.0);
    } else {
      report = pid_.Update(current_estimate - 0.8, dt.seconds());
    }
    if (true) {
      gpr_log(GPR_INFO, "RQ: pressure:%lf report:%lf error_integral:%lf",
              current_estimate, report, pid_.error_integral());
    }
    report_.store(report, std::memory_order_relaxed);
  });
  return report_.load(std::memory_order_relaxed);
}

}  // namespace memory_quota_detail

//
// MemoryQuota
//

MemoryAllocator MemoryQuota::CreateMemoryAllocator(absl::string_view name) {
  auto impl = std::make_shared<GrpcMemoryAllocatorImpl>(
      memory_quota_, absl::StrCat(memory_quota_->name(), "/allocator/", name));
  return MemoryAllocator(std::move(impl));
}

MemoryOwner MemoryQuota::CreateMemoryOwner(absl::string_view name) {
  auto impl = std::make_shared<GrpcMemoryAllocatorImpl>(
      memory_quota_, absl::StrCat(memory_quota_->name(), "/owner/", name));
  return MemoryOwner(std::move(impl));
}

}  // namespace grpc_core
