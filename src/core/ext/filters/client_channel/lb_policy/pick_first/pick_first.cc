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

#include <grpc/support/port_platform.h>

#include <string.h>

#include <grpc/support/alloc.h>

#include "src/core/ext/filters/client_channel/lb_policy/subchannel_list.h"
#include "src/core/ext/filters/client_channel/lb_policy_registry.h"
#include "src/core/ext/filters/client_channel/server_address.h"
#include "src/core/ext/filters/client_channel/subchannel.h"
#include "src/core/lib/address_utils/sockaddr_utils.h"
#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/gprpp/sync.h"
#include "src/core/lib/transport/connectivity_state.h"
#include "src/core/lib/transport/error_utils.h"

namespace grpc_core {

TraceFlag grpc_lb_pick_first_trace(false, "pick_first");

namespace {

//
// pick_first LB policy
//

constexpr char kPickFirst[] = "pick_first";

class PickFirst : public LoadBalancingPolicy {
 public:
  explicit PickFirst(Args args);

  const char* name() const override { return kPickFirst; }

  void UpdateLocked(UpdateArgs args) override;
  void ExitIdleLocked() override;
  void ResetBackoffLocked() override;

 private:
  ~PickFirst() override;

  class PickFirstSubchannelList;

  class PickFirstSubchannelData
      : public SubchannelData<PickFirstSubchannelList,
                              PickFirstSubchannelData> {
   public:
    PickFirstSubchannelData(
        SubchannelList<PickFirstSubchannelList, PickFirstSubchannelData>*
            subchannel_list,
        const ServerAddress& address,
        RefCountedPtr<SubchannelInterface> subchannel)
        : SubchannelData(subchannel_list, address, std::move(subchannel)) {}

    void ProcessConnectivityChangeLocked(
        absl::optional<grpc_connectivity_state> old_state,
        grpc_connectivity_state new_state) override;

    // Processes the connectivity change to READY for an unselected subchannel.
    void ProcessUnselectedReadyLocked();
  };

  class PickFirstSubchannelList
      : public SubchannelList<PickFirstSubchannelList,
                              PickFirstSubchannelData> {
   public:
    PickFirstSubchannelList(PickFirst* policy, TraceFlag* tracer,
                            ServerAddressList addresses,
                            const grpc_channel_args& args)
        : SubchannelList(policy, tracer, std::move(addresses),
                         policy->channel_control_helper(), args) {
      // Need to maintain a ref to the LB policy as long as we maintain
      // any references to subchannels, since the subchannels'
      // pollset_sets will include the LB policy's pollset_set.
      policy->Ref(DEBUG_LOCATION, "subchannel_list").release();
    }

    ~PickFirstSubchannelList() override {
      PickFirst* p = static_cast<PickFirst*>(policy());
      p->Unref(DEBUG_LOCATION, "subchannel_list");
    }

    bool in_transient_failure() const { return in_transient_failure_; }
    void set_in_transient_failure(bool in_transient_failure) {
      in_transient_failure_ = in_transient_failure;
    }

    bool AllSubchannelsSeenInitialState() {
      for (size_t i = 0; i < num_subchannels(); ++i) {
        if (!subchannel(i)->connectivity_state().has_value()) return false;
      }
      return true;
    }

   private:
    bool in_transient_failure_ = false;
  };

  class Picker : public SubchannelPicker {
   public:
    explicit Picker(RefCountedPtr<SubchannelInterface> subchannel)
        : subchannel_(std::move(subchannel)) {}

    PickResult Pick(PickArgs /*args*/) override {
      return PickResult::Complete(subchannel_);
    }

   private:
    RefCountedPtr<SubchannelInterface> subchannel_;
  };

  void ShutdownLocked() override;

  void AttemptToConnectUsingLatestUpdateArgsLocked();

  // Lateset update args.
  UpdateArgs latest_update_args_;
  // All our subchannels.
  OrphanablePtr<PickFirstSubchannelList> subchannel_list_;
  // Latest pending subchannel list.
  OrphanablePtr<PickFirstSubchannelList> latest_pending_subchannel_list_;
  // Selected subchannel in \a subchannel_list_.
  PickFirstSubchannelData* selected_ = nullptr;
  // Are we in IDLE state?
  bool idle_ = false;
  // Are we shut down?
  bool shutdown_ = false;
};

PickFirst::PickFirst(Args args) : LoadBalancingPolicy(std::move(args)) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_lb_pick_first_trace)) {
    gpr_log(GPR_INFO, "Pick First %p created.", this);
  }
}

PickFirst::~PickFirst() {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_lb_pick_first_trace)) {
    gpr_log(GPR_INFO, "Destroying Pick First %p", this);
  }
  GPR_ASSERT(subchannel_list_ == nullptr);
  GPR_ASSERT(latest_pending_subchannel_list_ == nullptr);
}

void PickFirst::ShutdownLocked() {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_lb_pick_first_trace)) {
    gpr_log(GPR_INFO, "Pick First %p Shutting down", this);
  }
  shutdown_ = true;
  subchannel_list_.reset();
  latest_pending_subchannel_list_.reset();
}

void PickFirst::ExitIdleLocked() {
  if (shutdown_) return;
  if (idle_) {
    if (GRPC_TRACE_FLAG_ENABLED(grpc_lb_pick_first_trace)) {
      gpr_log(GPR_INFO, "Pick First %p exiting idle", this);
    }
    idle_ = false;
    AttemptToConnectUsingLatestUpdateArgsLocked();
  }
}

void PickFirst::ResetBackoffLocked() {
  if (subchannel_list_ != nullptr) subchannel_list_->ResetBackoffLocked();
  if (latest_pending_subchannel_list_ != nullptr) {
    latest_pending_subchannel_list_->ResetBackoffLocked();
  }
}

void PickFirst::AttemptToConnectUsingLatestUpdateArgsLocked() {
  // Create a subchannel list from latest_update_args_.
  ServerAddressList addresses;
  if (latest_update_args_.addresses.ok()) {
    addresses = *latest_update_args_.addresses;
  }
  auto subchannel_list = MakeOrphanable<PickFirstSubchannelList>(
      this, &grpc_lb_pick_first_trace, std::move(addresses),
      *latest_update_args_.args);
  // Empty update or no valid subchannels.
  if (subchannel_list->num_subchannels() == 0) {
    // Unsubscribe from all current subchannels.
    subchannel_list_ = std::move(subchannel_list);  // Empty list.
    selected_ = nullptr;
    // Put the channel in TRANSIENT_FAILURE.
    absl::Status status =
        latest_update_args_.addresses.ok()
            ? absl::UnavailableError(absl::StrCat(
                  "empty address list: ", latest_update_args_.resolution_note))
            : latest_update_args_.addresses.status();
    channel_control_helper()->UpdateState(
        GRPC_CHANNEL_TRANSIENT_FAILURE, status,
        absl::make_unique<TransientFailurePicker>(status));
    return;
  }
  // Start watching connectivity state of all subchannels.
// FIXME: do this in SubchannelList ctor?
  for (size_t i = 0; i < subchannel_list->num_subchannels(); ++i) {
    subchannel_list->subchannel(i)->StartConnectivityWatchLocked();
  }
  // Store the subchannel list in either subchannel_list_ or
  // latest_pending_subchannel_list_.
  // Note that we do not start trying to connect to any subchannel here,
  // since we will wait until we see the initial connectivity state for all
  // subchannels before doing that.
  if (selected_ == nullptr) {
    // We don't yet have a selected subchannel, so replace the current
    // subchannel list immediately.
    subchannel_list_ = std::move(subchannel_list);
  } else {
    // We do have a selected subchannel (which means it's READY), so keep
    // using it until one of the subchannels in the new list reports READY.
    if (latest_pending_subchannel_list_ != nullptr) {
      if (GRPC_TRACE_FLAG_ENABLED(grpc_lb_pick_first_trace)) {
        gpr_log(GPR_INFO,
                "Pick First %p Shutting down latest pending subchannel list "
                "%p, about to be replaced by newer latest %p",
                this, latest_pending_subchannel_list_.get(),
                subchannel_list.get());
      }
    }
    latest_pending_subchannel_list_ = std::move(subchannel_list);
  }
}

void PickFirst::UpdateLocked(UpdateArgs args) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_lb_pick_first_trace)) {
    if (args.addresses.ok()) {
      gpr_log(GPR_INFO,
              "Pick First %p received update with %" PRIuPTR " addresses", this,
              args.addresses->size());
    } else {
      gpr_log(GPR_INFO, "Pick First %p received update with address error: %s",
              this, args.addresses.status().ToString().c_str());
    }
  }
  // Add GRPC_ARG_INHIBIT_HEALTH_CHECKING channel arg.
  grpc_arg new_arg = grpc_channel_arg_integer_create(
      const_cast<char*>(GRPC_ARG_INHIBIT_HEALTH_CHECKING), 1);
  const grpc_channel_args* new_args =
      grpc_channel_args_copy_and_add(args.args, &new_arg, 1);
  std::swap(new_args, args.args);
  grpc_channel_args_destroy(new_args);
  // If the update contains a resolver error and we have a previous update
  // that was not a resolver error, keep using the previous addresses.
  if (!args.addresses.ok() && latest_update_args_.config != nullptr) {
    args.addresses = std::move(latest_update_args_.addresses);
  }
  // Update latest_update_args_.
  latest_update_args_ = std::move(args);
  // If we are not in idle, start connection attempt immediately.
  // Otherwise, we defer the attempt into ExitIdleLocked().
  if (!idle_) {
    AttemptToConnectUsingLatestUpdateArgsLocked();
  }
}

void PickFirst::PickFirstSubchannelData::ProcessConnectivityChangeLocked(
    absl::optional<grpc_connectivity_state> old_state,
    grpc_connectivity_state new_state) {
  PickFirst* p = static_cast<PickFirst*>(subchannel_list()->policy());
  // The notification must be for a subchannel in either the current or
  // latest pending subchannel lists.
  GPR_ASSERT(subchannel_list() == p->subchannel_list_.get() ||
             subchannel_list() == p->latest_pending_subchannel_list_.get());
  GPR_ASSERT(new_state != GRPC_CHANNEL_SHUTDOWN);
  // Handle updates for the currently selected subchannel.
  if (p->selected_ == this) {
    if (GRPC_TRACE_FLAG_ENABLED(grpc_lb_pick_first_trace)) {
      gpr_log(GPR_INFO,
              "Pick First %p selected subchannel connectivity changed to %s", p,
              ConnectivityStateName(new_state));
    }
    // If the new state is anything other than READY and there is a
    // pending update, switch to the pending update.
    if (new_state != GRPC_CHANNEL_READY &&
        p->latest_pending_subchannel_list_ != nullptr) {
      if (GRPC_TRACE_FLAG_ENABLED(grpc_lb_pick_first_trace)) {
        gpr_log(GPR_INFO,
                "Pick First %p promoting pending subchannel list %p to "
                "replace %p",
                p, p->latest_pending_subchannel_list_.get(),
                p->subchannel_list_.get());
      }
      p->selected_ = nullptr;
      CancelConnectivityWatchLocked(
          "selected subchannel failed; switching to pending update");
      p->subchannel_list_ = std::move(p->latest_pending_subchannel_list_);
      // Set our state to that of the pending subchannel list.
      if (p->subchannel_list_->in_transient_failure()) {
        absl::Status status = absl::UnavailableError(
            "selected subchannel failed; switching to pending update");
        p->channel_control_helper()->UpdateState(
            GRPC_CHANNEL_TRANSIENT_FAILURE, status,
            absl::make_unique<TransientFailurePicker>(status));
      } else {
        p->channel_control_helper()->UpdateState(
            GRPC_CHANNEL_CONNECTING, absl::Status(),
            absl::make_unique<QueuePicker>(
                p->Ref(DEBUG_LOCATION, "QueuePicker")));
      }
    } else {
      if (new_state == GRPC_CHANNEL_TRANSIENT_FAILURE) {
        // If the selected subchannel goes bad, request a re-resolution. We
        // also set the channel state to IDLE. The reason is that if the new
        // state is TRANSIENT_FAILURE due to a GOAWAY reception we don't want
        // to connect to the re-resolved backends until we leave IDLE state.
        // TODO(qianchengz): We may want to request re-resolution in
        // ExitIdleLocked().
        p->idle_ = true;
        p->channel_control_helper()->RequestReresolution();
        p->selected_ = nullptr;
        p->subchannel_list_.reset();
        p->channel_control_helper()->UpdateState(
            GRPC_CHANNEL_IDLE, absl::Status(),
            absl::make_unique<QueuePicker>(
                p->Ref(DEBUG_LOCATION, "QueuePicker")));
      } else {
        // This is unlikely but can happen when a subchannel has been asked
        // to reconnect by a different channel and this channel has dropped
        // some connectivity state notifications.
        if (new_state == GRPC_CHANNEL_READY) {
          p->channel_control_helper()->UpdateState(
              GRPC_CHANNEL_READY, absl::Status(),
              absl::make_unique<Picker>(subchannel()->Ref()));
        } else {  // CONNECTING
          p->channel_control_helper()->UpdateState(
              new_state, absl::Status(),
              absl::make_unique<QueuePicker>(
                  p->Ref(DEBUG_LOCATION, "QueuePicker")));
        }
      }
    }
    return;
  }
  // If we get here, there are two possible cases:
  // 1. We do not currently have a selected subchannel, and the update is
  //    for a subchannel in p->subchannel_list_ that we're trying to
  //    connect to.  The goal here is to find a subchannel that we can
  //    select.
  // 2. We do currently have a selected subchannel, and the update is
  //    for a subchannel in p->latest_pending_subchannel_list_.  The
  //    goal here is to find a subchannel from the update that we can
  //    select in place of the current one.
  subchannel_list()->set_in_transient_failure(false);
  // If the subchannel is READY, use it.
  if (new_state == GRPC_CHANNEL_READY) {
    ProcessUnselectedReadyLocked();
    return;
  }
  // If this is the initial connectivity state notification for this
  // subchannel and it's the last one we're waiting for, then start trying
  // to connect to the first subchannel.
  if (!old_state.has_value() &&
      subchannel_list()->AllSubchannelsSeenInitialState()) {
    subchannel_list()->subchannel(0)->subchannel()->AttemptToConnect();
    return;
  }
  // Otherwise, process connectivity state.
  switch (new_state) {
    case GRPC_CHANNEL_READY:
      // Already handled this case above, so this should not happen.
      GPR_UNREACHABLE_CODE(break);
    case GRPC_CHANNEL_TRANSIENT_FAILURE: {
      PickFirstSubchannelData* sd = this;
      size_t next_index =
          (sd->Index() + 1) % subchannel_list()->num_subchannels();
      sd = subchannel_list()->subchannel(next_index);
      // If we're tried all subchannels, set state to TRANSIENT_FAILURE.
      if (sd->Index() == 0) {
        if (GRPC_TRACE_FLAG_ENABLED(grpc_lb_pick_first_trace)) {
          gpr_log(GPR_INFO,
                  "Pick First %p subchannel list %p failed to connect to "
                  "all subchannels",
                  p, subchannel_list());
        }
        subchannel_list()->set_in_transient_failure(true);
        // In case 2, swap to the new subchannel list.  This means reporting
        // TRANSIENT_FAILURE and dropping the existing (working) connection,
        // but we can't ignore what the control plane has told us.
        if (subchannel_list() == p->latest_pending_subchannel_list_.get()) {
          if (GRPC_TRACE_FLAG_ENABLED(grpc_lb_pick_first_trace)) {
            gpr_log(GPR_INFO,
                    "Pick First %p promoting pending subchannel list %p to "
                    "replace %p",
                    p, p->latest_pending_subchannel_list_.get(),
                    p->subchannel_list_.get());
          }
          p->subchannel_list_ = std::move(p->latest_pending_subchannel_list_);
        }
        // If this is the current subchannel list (either because we were
        // in case 1 or because we were in case 2 and just promoted it to
        // be the current list), re-resolve and report new state.
        if (subchannel_list() == p->subchannel_list_.get()) {
          p->channel_control_helper()->RequestReresolution();
          absl::Status status =
              absl::UnavailableError("failed to connect to all addresses");
          p->channel_control_helper()->UpdateState(
              GRPC_CHANNEL_TRANSIENT_FAILURE, status,
              absl::make_unique<TransientFailurePicker>(status));
        }
      }
      sd->subchannel()->AttemptToConnect();
      break;
    }
    case GRPC_CHANNEL_CONNECTING:
    case GRPC_CHANNEL_IDLE: {
      // Only update connectivity state in case 1.
      if (subchannel_list() == p->subchannel_list_.get()) {
        p->channel_control_helper()->UpdateState(
            GRPC_CHANNEL_CONNECTING, absl::Status(),
            absl::make_unique<QueuePicker>(
                p->Ref(DEBUG_LOCATION, "QueuePicker")));
      }
      break;
    }
    case GRPC_CHANNEL_SHUTDOWN:
      GPR_UNREACHABLE_CODE(break);
  }
}

void PickFirst::PickFirstSubchannelData::ProcessUnselectedReadyLocked() {
  PickFirst* p = static_cast<PickFirst*>(subchannel_list()->policy());
  // If we get here, there are two possible cases:
  // 1. We do not currently have a selected subchannel, and the update is
  //    for a subchannel in p->subchannel_list_ that we're trying to
  //    connect to.  The goal here is to find a subchannel that we can
  //    select.
  // 2. We do currently have a selected subchannel, and the update is
  //    for a subchannel in p->latest_pending_subchannel_list_.  The
  //    goal here is to find a subchannel from the update that we can
  //    select in place of the current one.
  GPR_ASSERT(subchannel_list() == p->subchannel_list_.get() ||
             subchannel_list() == p->latest_pending_subchannel_list_.get());
  // Case 2.  Promote p->latest_pending_subchannel_list_ to p->subchannel_list_.
  if (subchannel_list() == p->latest_pending_subchannel_list_.get()) {
    if (GRPC_TRACE_FLAG_ENABLED(grpc_lb_pick_first_trace)) {
      gpr_log(GPR_INFO,
              "Pick First %p promoting pending subchannel list %p to "
              "replace %p",
              p, p->latest_pending_subchannel_list_.get(),
              p->subchannel_list_.get());
    }
    p->subchannel_list_ = std::move(p->latest_pending_subchannel_list_);
  }
  // Cases 1 and 2.
  if (GRPC_TRACE_FLAG_ENABLED(grpc_lb_pick_first_trace)) {
    gpr_log(GPR_INFO, "Pick First %p selected subchannel %p", p, subchannel());
  }
  p->selected_ = this;
  p->channel_control_helper()->UpdateState(
      GRPC_CHANNEL_READY, absl::Status(),
      absl::make_unique<Picker>(subchannel()->Ref()));
  for (size_t i = 0; i < subchannel_list()->num_subchannels(); ++i) {
    if (i != Index()) {
      subchannel_list()->subchannel(i)->ShutdownLocked();
    }
  }
}

class PickFirstConfig : public LoadBalancingPolicy::Config {
 public:
  const char* name() const override { return kPickFirst; }
};

//
// factory
//

class PickFirstFactory : public LoadBalancingPolicyFactory {
 public:
  OrphanablePtr<LoadBalancingPolicy> CreateLoadBalancingPolicy(
      LoadBalancingPolicy::Args args) const override {
    return MakeOrphanable<PickFirst>(std::move(args));
  }

  const char* name() const override { return kPickFirst; }

  RefCountedPtr<LoadBalancingPolicy::Config> ParseLoadBalancingConfig(
      const Json& /*json*/, grpc_error_handle* /*error*/) const override {
    return MakeRefCounted<PickFirstConfig>();
  }
};

}  // namespace

}  // namespace grpc_core

void grpc_lb_policy_pick_first_init() {
  grpc_core::LoadBalancingPolicyRegistry::Builder::
      RegisterLoadBalancingPolicyFactory(
          absl::make_unique<grpc_core::PickFirstFactory>());
}

void grpc_lb_policy_pick_first_shutdown() {}
