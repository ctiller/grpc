//
// Copyright 2015 gRPC authors.
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

#ifndef GRPC_SRC_CORE_CLIENT_CHANNEL_CLIENT_CHANNEL_H
#define GRPC_SRC_CORE_CLIENT_CHANNEL_CLIENT_CHANNEL_H

#include <grpc/support/port_platform.h>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

#include "src/core/ext/filters/channel_idle/idle_filter_state.h"
#include "src/core/lib/gprpp/single_set_ptr.h"
#include "src/core/lib/promise/observable.h"
#include "src/core/lib/surface/channel_interface.h"
#include "src/core/lib/transport/call_destination.h"
#include "src/core/lib/transport/call_filters.h"

namespace grpc_core {

class ClientChannel : public ChannelInterface {
 public:
  ClientChannel(absl::string_view target_uri, ChannelArgs args);

  ~ClientChannel() override;

  void Orphan() override;

  absl::string_view target() const override;

  void GetChannelInfo(const grpc_channel_info* channel_info) override;

  // Returns the current connectivity state.  If try_to_connect is true,
  // triggers a connection attempt if not already connected.
  grpc_connectivity_state CheckConnectivityState(bool try_to_connect) override;

  // Starts and stops a connectivity watch.  The watcher will be initially
  // notified as soon as the state changes from initial_state and then on
  // every subsequent state change until either the watch is stopped or
  // it is notified that the state has changed to SHUTDOWN.
  //
  // This is intended to be used when starting watches from code inside of
  // C-core (e.g., for a nested control plane channel for things like xds).
  void AddConnectivityWatcher(
      grpc_connectivity_state initial_state,
      OrphanablePtr<AsyncConnectivityStateWatcherInterface> watcher) override;
  void RemoveConnectivityWatcher(
      AsyncConnectivityStateWatcherInterface* watcher) override;

  void ResetConnectionBackoff() override;

  void SendPing(absl::AnyInvocable<void(absl::Status)> on_initiate,
                absl::AnyInvocable<void(absl::Status)> on_ack) override;

  // Creates a call on the channel.
  CallInitiator CreateCall(ClientMetadataHandle client_initial_metadata,
                           Arena* arena) override;

  // Creates a load balanced call on the channel.
  CallInitiator CreateLoadBalancedCall(
      ClientInitialMetadata client_initial_metadata,
      absl::AnyInvocable<void()> on_commit, bool is_transparent_retry);

  // Flag that this object gets stored in channel args as a raw pointer.
  struct RawPointerChannelArgTag {};
  static absl::string_view ChannelArgName() {
    return "grpc.internal.client_channel";
  }

 private:
  class ResolverResultHandler;
  class ClientChannelControlHelper;
  class SubchannelWrapper;

  void CreateResolverLocked() ABSL_EXCLUSIVE_LOCKS_REQUIRED(*work_serializer_);
  void DestroyResolverAndLbPolicyLocked()
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(*work_serializer_);

  void TryToConnectLocked() ABSL_EXCLUSIVE_LOCKS_REQUIRED(*work_serializer_);

  void OnResolverResultChangedLocked(Resolver::Result result)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(*work_serializer_);
  void OnResolverErrorLocked(absl::Status status)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(*work_serializer_);

  absl::Status CreateOrUpdateLbPolicyLocked(
      RefCountedPtr<LoadBalancingPolicy::Config> lb_policy_config,
      const absl::optional<std::string>& health_check_service_name,
      Resolver::Result result) ABSL_EXCLUSIVE_LOCKS_REQUIRED(*work_serializer_);
  OrphanablePtr<LoadBalancingPolicy> CreateLbPolicyLocked(
      const ChannelArgs& args) ABSL_EXCLUSIVE_LOCKS_REQUIRED(*work_serializer_);

  void UpdateServiceConfigInControlPlaneLocked(
      RefCountedPtr<ServiceConfig> service_config,
      RefCountedPtr<ConfigSelector> config_selector, std::string lb_policy_name)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(*work_serializer_);

  void UpdateServiceConfigInDataPlaneLocked()
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(*work_serializer_);

  void UpdateStateLocked(grpc_connectivity_state state,
                         const absl::Status& status, const char* reason)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(*work_serializer_);

  void UpdateStateAndPickerLocked(
      grpc_connectivity_state state, const absl::Status& status,
      const char* reason,
      RefCountedPtr<LoadBalancingPolicy::SubchannelPicker> picker)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(*work_serializer_);

  void StartIdleTimer();

  // Applies service config settings from config_selector to the call.
  // May modify call context and client_initial_metadata.
  absl::Status ApplyServiceConfigToCall(
      ConfigSelector& config_selector,
      ClientMetadataHandle& client_initial_metadata) const;

  // Does an LB pick for a call.  Returns one of the following things:
  // - Continue{}, meaning to queue the pick
  // - a non-OK status, meaning to fail the call
  // - a connected subchannel, meaning that the pick is complete
  // When the pick is complete, pushes client_initial_metadata onto
  // call_initiator.  Also adds the subchannel call tracker (if any) to
  // context.
  LoopCtl<absl::StatusOr<RefCountedPtr<ConnectedSubchannel>>> PickSubchannel(
      LoadBalancingPolicy::SubchannelPicker& picker,
      ClientMetadataHandle& client_initial_metadata,
      CallInitiator& call_initiator);

  //
  // Fields set at construction and never modified.
  //
  ChannelArgs channel_args_;
  ClientChannelFactory* client_channel_factory_;
  const size_t service_config_parser_index_;
  RefCountedPtr<ServiceConfig> default_service_config_;
  std::string uri_to_resolve_;
  std::string default_authority_;
  channelz::ChannelNode* channelz_node_;
  OrphanablePtr<CallDestination> call_destination_;

  //
  // State for LB calls.
  //
  CallSizeEstimator lb_call_size_estimator_;
  MemoryAllocator lb_call_allocator_;

  //
  // Idleness state.
  //
  const Duration idle_timeout_;
  IdleFilterState idle_state_(false);
  SingleSetPtr<Activity, typename ActivityPtr::deleter_type> idle_activity_;

  //
  // Fields related to name resolution.
  //
  struct ResolverDataForCalls {
    RefCountedPtr<ConfigSelector> config_selector;
    RefCountedPtr<CallFilters::Stack> filter_stack;
  };
  Observable<absl::StatusOr<ResolverDataForCalls>> resolver_data_for_calls_;

  //
  // Fields related to LB picks.
  //
  Observable<RefCountedPtr<LoadBalancingPolicy::SubchannelPicker>> picker_;

  //
  // Fields used in the control plane.  Guarded by work_serializer.
  //
  std::shared_ptr<WorkSerializer> work_serializer_;
  ConnectivityStateTracker state_tracker_ ABSL_GUARDED_BY(*work_serializer_);
  OrphanablePtr<Resolver> resolver_ ABSL_GUARDED_BY(*work_serializer_);
  bool previous_resolution_contained_addresses_
      ABSL_GUARDED_BY(*work_serializer_) = false;
  RefCountedPtr<ServiceConfig> saved_service_config_
      ABSL_GUARDED_BY(*work_serializer_);
  RefCountedPtr<ConfigSelector> saved_config_selector_
      ABSL_GUARDED_BY(*work_serializer_);
  OrphanablePtr<LoadBalancingPolicy> lb_policy_
      ABSL_GUARDED_BY(*work_serializer_);
  RefCountedPtr<SubchannelPoolInterface> subchannel_pool_
      ABSL_GUARDED_BY(*work_serializer_);
  // The number of SubchannelWrapper instances referencing a given Subchannel.
  std::map<Subchannel*, int> subchannel_refcount_map_
      ABSL_GUARDED_BY(*work_serializer_);
  // The set of SubchannelWrappers that currently exist.
  // No need to hold a ref, since the set is updated in the control-plane
  // work_serializer when the SubchannelWrappers are created and destroyed.
  absl::flat_hash_set<SubchannelWrapper*> subchannel_wrappers_
      ABSL_GUARDED_BY(*work_serializer_);
  int keepalive_time_ ABSL_GUARDED_BY(*work_serializer_) = -1;
  absl::Status disconnect_error_ ABSL_GUARDED_BY(*work_serializer_);

  //
  // Fields accessed via GetChannelInfo().
  //
  Mutex info_mu_;
  std::string info_lb_policy_name_ ABSL_GUARDED_BY(info_mu_);
  std::string info_service_config_json_ ABSL_GUARDED_BY(info_mu_);
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_CLIENT_CHANNEL_CLIENT_CHANNEL_H
