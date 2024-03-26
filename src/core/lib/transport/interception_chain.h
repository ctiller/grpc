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

#ifndef GRPC_SRC_CORE_LIB_TRANSPORT_INTERCEPTION_CHAIN_H
#define GRPC_SRC_CORE_LIB_TRANSPORT_INTERCEPTION_CHAIN_H

#include <grpc/support/port_platform.h>

#include <memory>
#include <vector>

#include "src/core/lib/gprpp/ref_counted.h"
#include "src/core/lib/transport/call_destination.h"
#include "src/core/lib/transport/call_filters.h"
#include "src/core/lib/transport/call_spine.h"
#include "src/core/lib/transport/metadata.h"

namespace grpc_core {

class InterceptionChainBuilder;

namespace interception_chain_detail {

class HijackedCall {
 public:
  HijackedCall(ClientMetadataHandle metadata,
               RefCountedPtr<UnstartedCallDestination> destination,
               CallHandler call_handler)
      : metadata_(std::move(metadata)),
        destination_(std::move(destination)),
        call_handler_(std::move(call_handler)) {}

  CallInitiator MakeCall();
  CallInitiator MakeLastCall() {
    return MakeCallWithMetadata(std::move(metadata_));
  }

  CallHandler& original_call_handler() { return call_handler_; }

 private:
  CallInitiator MakeCallWithMetadata(ClientMetadataHandle metadata);

  ClientMetadataHandle metadata_;
  RefCountedPtr<UnstartedCallDestination> destination_;
  CallHandler call_handler_;
};

inline auto HijackCall(UnstartedCallHandler unstarted_call_handler,
                       RefCountedPtr<UnstartedCallDestination> destination,
                       RefCountedPtr<CallFilters::Stack> stack) {
  auto call_handler = unstarted_call_handler.StartCall(stack);
  return Map(
      call_handler.PullClientInitialMetadata(),
      [call_handler,
       destination](ValueOrFailure<ClientMetadataHandle> metadata) mutable
      -> ValueOrFailure<HijackedCall> {
        if (!metadata.ok()) return Failure{};
        return HijackedCall(std::move(metadata.value()), std::move(destination),
                            std::move(call_handler));
      });
}

}  // namespace interception_chain_detail

// A delegating UnstartedCallDestination for use as a hijacking filter.
// Implementations may look at the unprocessed initial metadata
// and decide to do one of two things:
//
// 1. It can hijack the call. Returns a HijackedCall object that can
//    be used to start new calls with the same metadata.
//
// 2. It can consume the call by calling `Consume`.
class Interceptor : public UnstartedCallDestination {
 protected:
  using HijackedCall = interception_chain_detail::HijackedCall;

  auto Hijack(UnstartedCallHandler unstarted_call_handler) {
    return interception_chain_detail::HijackCall(
        std::move(unstarted_call_handler), wrapped_destination_, filter_stack_);
  }

  CallHandler Consume(UnstartedCallHandler unstarted_call_handler) {
    return unstarted_call_handler.StartCall(filter_stack_);
  }

 private:
  friend class InterceptionChainBuilder;

  RefCountedPtr<UnstartedCallDestination> wrapped_destination_;
  RefCountedPtr<CallFilters::Stack> filter_stack_;
};

class InterceptionChainBuilder final {
 public:
  // The kind of destination that the chain will eventually call.
  // We can bottom out in various types depending on where we're intercepting:
  // - The top half of the client channel wants to terminate on a
  //   UnstartedCallDestination (specifically the LB call destination).
  // - The bottom half of the client channel and the server code wants to
  //   terminate on a ClientTransport - which unlike a
  //   UnstartedCallDestination demands a started CallHandler.
  // There's some adaption code that's needed to start filters just prior
  // to the bottoming out, and some design considerations to make with that.
  // One way (that's not chosen here) would be to have the caller of the
  // Builder provide something that can build an adaptor
  // UnstartedCallDestination with parameters supplied by this builder - that
  // disperses the responsibility of building the adaptor to the caller, which
  // is not ideal - we might want to adjust the way this construct is built in
  // the future, and building is a builder responsibility.
  // Instead, we declare a relatively closed set of destinations here, and
  // hide the adaptors inside the builder at build time.
  using FinalDestination =
      absl::variant<RefCountedPtr<UnstartedCallDestination>,
                    RefCountedPtr<CallDestination>>;

  explicit InterceptionChainBuilder(ChannelArgs args)
      : args_(std::move(args)) {}

  // Add a filter with a `Call` class as an inner member.
  // Call class must be one compatible with the filters described in
  // call_filters.h.
  template <typename T>
  absl::enable_if_t<sizeof(typename T::Call) != 0, InterceptionChainBuilder&>
  Add() {
    if (!status_.ok()) return *this;
    auto filter = T::Create(args_);
    if (!filter.ok()) {
      status_ = filter.status();
      return *this;
    }
    auto& sb = stack_builder();
    sb.Add(filter.value().get());
    sb.AddOwnedObject(std::move(filter.value()));
    return *this;
  };

  // Add a filter that is an interceptor - one that can hijack calls.
  template <typename T>
  absl::enable_if_t<std::is_base_of<Interceptor, T>::value,
                    InterceptionChainBuilder&>
  Add() {
    AddInterceptor(T::Create(args_));
    return *this;
  };

  template <typename F>
  void AddOnServerTrailingMetadata(F f) {
    stack_builder().AddOnServerTrailingMetadata(std::move(f));
  }

  absl::StatusOr<RefCountedPtr<UnstartedCallDestination>> Build(
      FinalDestination final_destination);

  const ChannelArgs& channel_args() const { return args_; }

 private:
  CallFilters::StackBuilder& stack_builder() {
    if (!stack_builder_.has_value()) stack_builder_.emplace();
    return *stack_builder_;
  }

  RefCountedPtr<CallFilters::Stack> MakeFilterStack() {
    auto stack = stack_builder().Build();
    stack_builder_.reset();
    return stack;
  }

  void AddInterceptor(absl::StatusOr<RefCountedPtr<Interceptor>> interceptor);

  ChannelArgs args_;
  absl::optional<CallFilters::StackBuilder> stack_builder_;
  RefCountedPtr<Interceptor> top_interceptor_;
  absl::Status status_;
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_TRANSPORT_INTERCEPTION_CHAIN_H
