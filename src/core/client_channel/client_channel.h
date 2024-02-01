//
// Copyright 2024 gRPC authors.
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

#ifndef GRPC_SRC_CORE_LIB_CLIENT_CHANNEL_CLIENT_CHANNEL_H
#define GRPC_SRC_CORE_LIB_CLIENT_CHANNEL_CLIENT_CHANNEL_H

#include <grpc/support/port_platform.h>

#include "src/core/lib/promise/observable.h"
#include "src/core/lib/surface/channel.h"
#include "src/core/lib/transport/call_factory.h"
#include "src/core/lib/transport/call_filters.h"

namespace grpc_core {

class ClientChannel : public CallFactory {
 public:
  class LoadBalancedChannel;

  ClientChannel(absl::string_view target_uri, ChannelArgs args);

  // Returns the ClientChannel object from channel, or null if channel
  // is not a client channel.
  static ClientChannel* GetFromChannel(Channel* channel);

  // Creates a call on the channel.
  CallInitiator CreateCall(ClientMetadataHandle md, Arena* arena) override;

  // Creates a load balanced call on the channel.
  CallInitiator CreateLoadBalancedCall(
      ClientMetadataHandle md, absl::AnyInvocable<void()> on_commit,
      bool is_transparent_retry);

  // Returns the current connectivity state.  If try_to_connect is true,
  // triggers a connection attempt if not already connected.
  grpc_connectivity_state CheckConnectivityState(bool try_to_connect);

#if 0
  // Starts a one-time connectivity state watch.  When the channel's state
  // becomes different from *state, sets *state to the new state and
  // schedules on_complete.  The watcher_timer_init callback is invoked as
  // soon as the watch is actually started (i.e., after hopping into the
  // client channel combiner).  I/O will be serviced via pollent.
  //
  // This is intended to be used when starting a watch from outside of C-core
  // via grpc_channel_watch_connectivity_state().  It should not be used
  // by other callers.
  void AddExternalConnectivityWatcher(grpc_polling_entity pollent,
                                      grpc_connectivity_state* state,
                                      grpc_closure* on_complete,
                                      grpc_closure* watcher_timer_init) {
    new ExternalConnectivityWatcher(this, pollent, state, on_complete,
                                    watcher_timer_init);
  }

  // Cancels a pending external watcher previously added by
  // AddExternalConnectivityWatcher().
  void CancelExternalConnectivityWatcher(grpc_closure* on_complete) {
    ExternalConnectivityWatcher::RemoveWatcherFromExternalWatchersMap(
        this, on_complete, /*cancel=*/true);
  }

  int NumExternalConnectivityWatchers() const {
    MutexLock lock(&external_watchers_mu_);
    return static_cast<int>(external_watchers_.size());
  }

  // Starts and stops a connectivity watch.  The watcher will be initially
  // notified as soon as the state changes from initial_state and then on
  // every subsequent state change until either the watch is stopped or
  // it is notified that the state has changed to SHUTDOWN.
  //
  // This is intended to be used when starting watches from code inside of
  // C-core (e.g., for a nested control plane channel for things like xds).
  void AddConnectivityWatcher(
      grpc_connectivity_state initial_state,
      OrphanablePtr<AsyncConnectivityStateWatcherInterface> watcher);
  void RemoveConnectivityWatcher(
      AsyncConnectivityStateWatcherInterface* watcher);
#endif

 private:
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

  //
  // Fields related to name resolution.
  //
  struct ResolverDataForCalls {
    RefCountedPtr<ConfigSelector> config_selector;
    RefCountedPtr<CallFilters::Stack> filter_stack;
  };
  Observable<StatusOr<ResolverDataForCalls>> resolver_data_for_calls_;

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
  // Fields accessed via get_channel_info().
  //
  Mutex info_mu_;
  std::string info_lb_policy_name_ ABSL_GUARDED_BY(info_mu_);
  std::string info_service_config_json_ ABSL_GUARDED_BY(info_mu_);

#if 0
  //
  // Fields guarded by a mutex, since they need to be accessed
  // synchronously via grpc_channel_num_external_connectivity_watchers().
  //
  mutable Mutex external_watchers_mu_;
  std::map<grpc_closure*, RefCountedPtr<ExternalConnectivityWatcher>>
      external_watchers_ ABSL_GUARDED_BY(external_watchers_mu_);
#endif
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_CLIENT_CHANNEL_CLIENT_CHANNEL_H