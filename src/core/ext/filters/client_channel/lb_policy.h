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

#ifndef GRPC_CORE_EXT_FILTERS_CLIENT_CHANNEL_LB_POLICY_H
#define GRPC_CORE_EXT_FILTERS_CLIENT_CHANNEL_LB_POLICY_H

#include <grpc/support/port_platform.h>

#include <functional>
#include <iterator>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "absl/types/variant.h"

#include "src/core/ext/filters/client_channel/server_address.h"
#include "src/core/ext/filters/client_channel/service_config.h"
#include "src/core/ext/filters/client_channel/subchannel_interface.h"
#include "src/core/lib/gprpp/orphanable.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/iomgr/polling_entity.h"
#include "src/core/lib/iomgr/work_serializer.h"
#include "src/core/lib/transport/connectivity_state.h"

namespace grpc_core {

extern DebugOnlyTraceFlag grpc_trace_lb_policy_refcount;

/// Interface for load balancing policies.
///
/// The following concepts are used here:
///
/// Channel: An abstraction that manages connections to backend servers
///   on behalf of a client application.  The application creates a channel
///   for a given server name and then sends calls (RPCs) on it, and the
///   channel figures out which backend server to send each call to.  A channel
///   contains a resolver, a load balancing policy (or a tree of LB policies),
///   and a set of one or more subchannels.
///
/// Subchannel: A subchannel represents a connection to one backend server.
///   The LB policy decides which subchannels to create, manages the
///   connectivity state of those subchannels, and decides which subchannel
///   to send any given call to.
///
/// Resolver: A plugin that takes a gRPC server URI and resolves it to a
///   list of one or more addresses and a service config, as described
///   in https://github.com/grpc/grpc/blob/master/doc/naming.md.  See
///   resolver.h for the resolver API.
///
/// Load Balancing (LB) Policy: A plugin that takes a list of addresses
///   from the resolver, maintains and manages a subchannel for each
///   backend address, and decides which subchannel to send each call on.
///   An LB policy has two parts:
///   - A LoadBalancingPolicy, which deals with the control plane work of
///     managing subchannels.
///   - A SubchannelPicker, which handles the data plane work of
///     determining which subchannel a given call should be sent on.

/// LoadBalacingPolicy API.
///
/// Note: All methods with a "Locked" suffix must be called from the
/// work_serializer passed to the constructor.
///
/// Any I/O done by the LB policy should be done under the pollset_set
/// returned by \a interested_parties().
// TODO(roth): Once we move to EventManager-based polling, remove the
// interested_parties() hooks from the API.
class LoadBalancingPolicy : public InternallyRefCounted<LoadBalancingPolicy> {
 public:
  // Represents backend metrics reported by the backend to the client.
  struct BackendMetricData {
    /// CPU utilization expressed as a fraction of available CPU resources.
    double cpu_utilization;
    /// Memory utilization expressed as a fraction of available memory
    /// resources.
    double mem_utilization;
    /// Total requests per second being served by the backend.  This
    /// should include all services that a backend is responsible for.
    uint64_t requests_per_second;
    /// Application-specific requests cost metrics.  Metric names are
    /// determined by the application.  Each value is an absolute cost
    /// (e.g. 3487 bytes of storage) associated with the request.
    std::map<absl::string_view, double> request_cost;
    /// Application-specific resource utilization metrics.  Metric names
    /// are determined by the application.  Each value is expressed as a
    /// fraction of total resources available.
    std::map<absl::string_view, double> utilization;
  };

  /// Interface for accessing per-call state.
  /// Implemented by the client channel and used by the SubchannelPicker.
  class CallState {
   public:
    CallState() = default;
    virtual ~CallState() = default;

    /// Allocates memory associated with the call, which will be
    /// automatically freed when the call is complete.
    /// It is more efficient to use this than to allocate memory directly
    /// for allocations that need to be made on a per-call basis.
    virtual void* Alloc(size_t size) = 0;

    /// Returns the backend metric data returned by the server for the call,
    /// or null if no backend metric data was returned.
    // TODO(roth): Move this out of CallState, since it should not be
    // accessible to the picker, only to the recv_trailing_metadata_ready
    // callback.  It should instead be in its own interface.
    virtual const BackendMetricData* GetBackendMetricData() = 0;

    /// EXPERIMENTAL API.
    /// Returns the value of the call attribute \a key.
    /// Keys are static strings, so an attribute can be accessed by an LB
    /// policy implementation only if it knows about the internal key.
    /// Returns a null string_view if key not found.
    virtual absl::string_view ExperimentalGetCallAttribute(const char* key) = 0;
  };

  /// Interface for accessing metadata.
  /// Implemented by the client channel and used by the SubchannelPicker.
  class MetadataInterface {
   public:
    class iterator
        : public std::iterator<
              std::input_iterator_tag,
              std::pair<absl::string_view, absl::string_view>,  // value_type
              std::ptrdiff_t,  // difference_type
              std::pair<absl::string_view, absl::string_view>*,  // pointer
              std::pair<absl::string_view, absl::string_view>&   // reference
              > {
     public:
      iterator(const MetadataInterface* md, intptr_t handle)
          : md_(md), handle_(handle) {}
      iterator& operator++() {
        handle_ = md_->IteratorHandleNext(handle_);
        return *this;
      }
      bool operator==(iterator other) const {
        return md_ == other.md_ && handle_ == other.handle_;
      }
      bool operator!=(iterator other) const { return !(*this == other); }
      value_type operator*() const { return md_->IteratorHandleGet(handle_); }

     private:
      friend class MetadataInterface;
      const MetadataInterface* md_;
      intptr_t handle_;
    };

    virtual ~MetadataInterface() = default;

    /// Adds a key/value pair.
    /// Does NOT take ownership of \a key or \a value.
    /// Implementations must ensure that the key and value remain alive
    /// until the call ends.  If desired, they may be allocated via
    /// CallState::Alloc().
    virtual void Add(absl::string_view key, absl::string_view value) = 0;

    /// Iteration interface.
    virtual iterator begin() const = 0;
    virtual iterator end() const = 0;

    /// Removes the element pointed to by \a it.
    /// Returns an iterator pointing to the next element.
    virtual iterator erase(iterator it) = 0;

   protected:
    intptr_t GetIteratorHandle(const iterator& it) const { return it.handle_; }

   private:
    friend class iterator;

    virtual intptr_t IteratorHandleNext(intptr_t handle) const = 0;
    virtual std::pair<absl::string_view /*key*/, absl::string_view /*value */>
    IteratorHandleGet(intptr_t handle) const = 0;
  };

  /// Arguments used when picking a subchannel for a call.
  struct PickArgs {
    /// The path of the call.  Indicates the RPC service and method name.
    absl::string_view path;
    /// Initial metadata associated with the picking call.
    /// The LB policy may use the existing metadata to influence its routing
    /// decision, and it may add new metadata elements to be sent with the
    /// call to the chosen backend.
    MetadataInterface* initial_metadata;
    /// An interface for accessing call state.  Can be used to allocate
    /// memory associated with the call in an efficient way.
    CallState* call_state;
  };

  /// The result of picking a subchannel for a call.
  struct PickResult {
    /// A successful pick.
    struct Complete {
      /// The subchannel to be used for the call.  Must be non-null.
      RefCountedPtr<SubchannelInterface> subchannel;

      /// Callback set by LB policy to be notified of trailing metadata.
      /// If non-null, the client channel will invoke the callback
      /// when trailing metadata is returned.
      /// The metadata may be modified by the callback.  However, the callback
      /// does not take ownership, so any data that needs to be used after
      /// returning must be copied.
      /// The call state can be used to obtain backend metric data.
      // TODO(roth): The arguments to this callback should be moved into a
      // struct, so that we can later add new fields without breaking
      // existing implementations.
      std::function<void(absl::Status, MetadataInterface*, CallState*)>
          recv_trailing_metadata_ready;

      explicit Complete(
          RefCountedPtr<SubchannelInterface> sc,
          std::function<void(absl::Status, MetadataInterface*, CallState*)> cb =
              nullptr)
          : subchannel(std::move(sc)),
            recv_trailing_metadata_ready(std::move(cb)) {}
    };

    /// Pick cannot be completed until something changes on the control
    /// plane.  The client channel will queue the pick and try again the
    /// next time the picker is updated.
    struct Queue {};

    /// Pick failed.  If the call is wait_for_ready, the client channel
    /// will wait for the next picker and try again; otherwise, it
    /// will immediately fail the call with the status indicated (although
    /// the call may be retried if the client channel is configured to do so).
    struct Fail {
      absl::Status status;

      explicit Fail(absl::Status s) : status(s) {}
    };

    /// Pick will be dropped with the status specified.
    /// Unlike FailPick, the call will be dropped even if it is
    /// wait_for_ready, and retries (if configured) will be inhibited.
    struct Drop {
      absl::Status status;

      explicit Drop(absl::Status s) : status(s) {}
    };

    // A pick result must be one of these types.
    // Default to Queue, just to allow default construction.
    absl::variant<Complete, Queue, Fail, Drop> result = Queue();

    PickResult() = default;
    // NOLINTNEXTLINE(google-explicit-constructor)
    PickResult(Complete complete) : result(std::move(complete)) {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    PickResult(Queue queue) : result(queue) {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    PickResult(Fail fail) : result(std::move(fail)) {}
    // NOLINTNEXTLINE(google-explicit-constructor)
    PickResult(Drop drop) : result(std::move(drop)) {}

    // Convenience function for evaluating a PickResult.
    // Calls the appropriate function for the type of result.
    template <typename T>
    T Handle(std::function<T(Complete&)> complete_func,
             std::function<T(Queue&)> queue_func,
             std::function<T(Fail&)> fail_func,
             std::function<T(Drop&)> drop_func);
  };

  /// A subchannel picker is the object used to pick the subchannel to
  /// use for a given call.  This is implemented by the LB policy and
  /// used by the client channel to perform picks.
  ///
  /// Pickers are intended to encapsulate all of the state and logic
  /// needed on the data plane (i.e., to actually process picks for
  /// individual calls sent on the channel) while excluding all of the
  /// state and logic needed on the control plane (i.e., resolver
  /// updates, connectivity state notifications, etc); the latter should
  /// live in the LB policy object itself.
  ///
  /// Currently, pickers are always accessed from within the
  /// client_channel data plane mutex, so they do not have to be
  /// thread-safe.
  class SubchannelPicker {
   public:
    SubchannelPicker() = default;
    virtual ~SubchannelPicker() = default;

    virtual PickResult Pick(PickArgs args) = 0;
  };

  /// A proxy object implemented by the client channel and used by the
  /// LB policy to communicate with the channel.
  class ChannelControlHelper {
   public:
    ChannelControlHelper() = default;
    virtual ~ChannelControlHelper() = default;

    /// Creates a new subchannel with the specified channel args.
    virtual RefCountedPtr<SubchannelInterface> CreateSubchannel(
        ServerAddress address, const grpc_channel_args& args) = 0;

    /// Sets the connectivity state and returns a new picker to be used
    /// by the client channel.
    virtual void UpdateState(grpc_connectivity_state state,
                             const absl::Status& status,
                             std::unique_ptr<SubchannelPicker>) = 0;

    /// Requests that the resolver re-resolve.
    virtual void RequestReresolution() = 0;

    /// Adds a trace message associated with the channel.
    enum TraceSeverity { TRACE_INFO, TRACE_WARNING, TRACE_ERROR };
    virtual void AddTraceEvent(TraceSeverity severity,
                               absl::string_view message) = 0;
  };

  /// Interface for configuration data used by an LB policy implementation.
  /// Individual implementations will create a subclass that adds methods to
  /// return the parameters they need.
  class Config : public RefCounted<Config> {
   public:
    ~Config() override = default;

    // Returns the load balancing policy name
    virtual const char* name() const = 0;
  };

  /// Data passed to the UpdateLocked() method when new addresses and
  /// config are available.
  struct UpdateArgs {
    ServerAddressList addresses;
    RefCountedPtr<Config> config;
    const grpc_channel_args* args = nullptr;

    // TODO(roth): Remove everything below once channel args is
    // converted to a copyable and movable C++ object.
    UpdateArgs() = default;
    ~UpdateArgs() { grpc_channel_args_destroy(args); }
    UpdateArgs(const UpdateArgs& other);
    UpdateArgs(UpdateArgs&& other) noexcept;
    UpdateArgs& operator=(const UpdateArgs& other);
    UpdateArgs& operator=(UpdateArgs&& other) noexcept;
  };

  /// Args used to instantiate an LB policy.
  struct Args {
    /// The work_serializer under which all LB policy calls will be run.
    std::shared_ptr<WorkSerializer> work_serializer;
    /// Channel control helper.
    /// Note: LB policies MUST NOT call any method on the helper from
    /// their constructor.
    std::unique_ptr<ChannelControlHelper> channel_control_helper;
    /// Channel args.
    // TODO(roth): Find a better channel args representation for this API.
    // TODO(roth): Clarify ownership semantics here -- currently, this
    // does not take ownership of args, which is the opposite of how we
    // handle them in UpdateArgs.
    const grpc_channel_args* args = nullptr;
  };

  explicit LoadBalancingPolicy(Args args, intptr_t initial_refcount = 1);
  ~LoadBalancingPolicy() override;

  // Not copyable nor movable.
  LoadBalancingPolicy(const LoadBalancingPolicy&) = delete;
  LoadBalancingPolicy& operator=(const LoadBalancingPolicy&) = delete;

  /// Returns the name of the LB policy.
  virtual const char* name() const = 0;

  /// Updates the policy with new data from the resolver.  Will be invoked
  /// immediately after LB policy is constructed, and then again whenever
  /// the resolver returns a new result.
  virtual void UpdateLocked(UpdateArgs) = 0;  // NOLINT

  /// Tries to enter a READY connectivity state.
  /// This is a no-op by default, since most LB policies never go into
  /// IDLE state.
  virtual void ExitIdleLocked() {}

  /// Resets connection backoff.
  virtual void ResetBackoffLocked() = 0;

  grpc_pollset_set* interested_parties() const { return interested_parties_; }

  // Note: This must be invoked while holding the work_serializer.
  void Orphan() override;

  // A picker that returns kQueue for all picks.
  // Also calls the parent LB policy's ExitIdleLocked() method when the
  // first pick is seen.
  class QueuePicker : public SubchannelPicker {
   public:
    explicit QueuePicker(RefCountedPtr<LoadBalancingPolicy> parent)
        : parent_(std::move(parent)) {}

    ~QueuePicker() override { parent_.reset(DEBUG_LOCATION, "QueuePicker"); }

    PickResult Pick(PickArgs args) override;

   private:
    RefCountedPtr<LoadBalancingPolicy> parent_;
    bool exit_idle_called_ = false;
  };

  // A picker that returns kFail for all picks.
  class TransientFailurePicker : public SubchannelPicker {
   public:
    explicit TransientFailurePicker(absl::Status status) : status_(status) {}

    PickResult Pick(PickArgs /*args*/) override {
      return PickResult::Fail(status_);
    }

   private:
    absl::Status status_;
  };

 protected:
  std::shared_ptr<WorkSerializer> work_serializer() const {
    return work_serializer_;
  }

  // Note: LB policies MUST NOT call any method on the helper from their
  // constructor.
  ChannelControlHelper* channel_control_helper() const {
    return channel_control_helper_.get();
  }

  /// Shuts down the policy.
  virtual void ShutdownLocked() = 0;

 private:
  /// Work Serializer under which LB policy actions take place.
  std::shared_ptr<WorkSerializer> work_serializer_;
  /// Owned pointer to interested parties in load balancing decisions.
  grpc_pollset_set* interested_parties_;
  /// Channel control helper.
  std::unique_ptr<ChannelControlHelper> channel_control_helper_;
};

//
// Implementation
//

template <typename T>
T LoadBalancingPolicy::PickResult::Handle(
    std::function<T(Complete&)> complete_func,
    std::function<T(Queue&)> queue_func, std::function<T(Fail&)> fail_func,
    std::function<T(Drop&)> drop_func) {
  auto* complete_pick = absl::get_if<Complete>(&result);
  if (complete_pick != nullptr) {
    return complete_func(*complete_pick);
  }
  auto* queue_pick = absl::get_if<Queue>(&result);
  if (queue_pick != nullptr) {
    return queue_func(*queue_pick);
  }
  auto* fail_pick = absl::get_if<Fail>(&result);
  if (fail_pick != nullptr) {
    return fail_func(*fail_pick);
  }
  auto* drop_pick = absl::get_if<Drop>(&result);
  GPR_ASSERT(drop_pick != nullptr);
  return drop_func(*drop_pick);
}

}  // namespace grpc_core

#endif /* GRPC_CORE_EXT_FILTERS_CLIENT_CHANNEL_LB_POLICY_H */
