//
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
//

#ifndef GRPC_SRC_CORE_EXT_FILTERS_FAULT_INJECTION_FAULT_INJECTION_FILTER_H
#define GRPC_SRC_CORE_EXT_FILTERS_FAULT_INJECTION_FAULT_INJECTION_FILTER_H

#include <grpc/support/port_platform.h>

#include <stddef.h>

#include <memory>

#include "absl/base/thread_annotations.h"
#include "absl/random/random.h"
#include "absl/status/statusor.h"

#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/channel/channel_fwd.h"
#include "src/core/lib/channel/promise_based_filter.h"
#include "src/core/lib/gprpp/sync.h"
#include "src/core/lib/promise/sleep.h"
#include "src/core/lib/promise/try_seq.h"
#include "src/core/lib/transport/transport.h"

namespace grpc_core {

extern TraceFlag grpc_fault_injection_filter_trace;

// This channel filter is intended to be used by the dynamic filters, instead
// of the ordinary channel stack. The fault injection filter fetches fault
// injection policy from the method config of service config returned by the
// resolver, and enforces the fault injection policy.
class FaultInjectionFilter
    : public ImplementChannelFilter<FaultInjectionFilter> {
 public:
  static const grpc_channel_filter kFilter;

  static absl::StatusOr<FaultInjectionFilter> Create(
      const ChannelArgs& args, ChannelFilter::Args filter_args);

  class Call {
   public:
    auto OnClientInitialMetadata(ClientMetadata& md,
                                 FaultInjectionFilter* filter) {
      auto decision = filter->MakeInjectionDecision(md);
      if (GRPC_TRACE_FLAG_ENABLED(grpc_fault_injection_filter_trace)) {
        gpr_log(GPR_INFO, "chand=%p: Fault injection triggered %s", this,
                decision.ToString().c_str());
      }
      auto delay = decision.DelayUntil();
      return TrySeq(Sleep(delay), [decision = std::move(decision)]() {
        return decision.MaybeAbort();
      });
    }
  };

 private:
  explicit FaultInjectionFilter(ChannelFilter::Args filter_args);

  // Tracks an active faults lifetime.
  // Increments g_active_faults when created, and decrements it when destroyed.
  class FaultHandle {
   public:
    explicit FaultHandle(bool active);
    ~FaultHandle();
    FaultHandle(const FaultHandle&) = delete;
    FaultHandle& operator=(const FaultHandle&) = delete;
    FaultHandle(FaultHandle&& other) noexcept
        : active_(std::exchange(other.active_, false)) {}
    FaultHandle& operator=(FaultHandle&& other) noexcept {
      std::swap(active_, other.active_);
      return *this;
    }

   private:
    bool active_;
  };

  class InjectionDecision {
   public:
    InjectionDecision(uint32_t max_faults, Duration delay_time,
                      absl::optional<absl::Status> abort_request)
        : max_faults_(max_faults),
          delay_time_(delay_time),
          abort_request_(abort_request) {}

    std::string ToString() const;
    Timestamp DelayUntil();
    absl::Status MaybeAbort() const;

   private:
    bool HaveActiveFaultsQuota() const;

    uint32_t max_faults_;
    Duration delay_time_;
    absl::optional<absl::Status> abort_request_;
    FaultHandle active_fault_{false};
  };
  InjectionDecision MakeInjectionDecision(
      const ClientMetadata& initial_metadata);

  // The relative index of instances of the same filter.
  size_t index_;
  const size_t service_config_parser_index_;
  std::unique_ptr<Mutex> mu_;
  absl::InsecureBitGen abort_rand_generator_ ABSL_GUARDED_BY(mu_);
  absl::InsecureBitGen delay_rand_generator_ ABSL_GUARDED_BY(mu_);
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_EXT_FILTERS_FAULT_INJECTION_FAULT_INJECTION_FILTER_H
