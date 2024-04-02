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

#include <grpc/support/port_platform.h>

#include "src/core/lib/transport/interception_chain.h"

#include "src/core/lib/gprpp/match.h"
#include "src/core/lib/transport/call_destination.h"
#include "src/core/lib/transport/call_filters.h"
#include "src/core/lib/transport/call_spine.h"
#include "src/core/lib/transport/metadata.h"

namespace grpc_core {

///////////////////////////////////////////////////////////////////////////////
// HijackedCall

namespace interception_chain_detail {

CallInitiator HijackedCall::MakeCall() {
  auto metadata = Arena::MakePooled<ClientMetadata>();
  *metadata = metadata_->Copy();
  return MakeCallWithMetadata(std::move(metadata));
}

CallInitiator HijackedCall::MakeCallWithMetadata(
    ClientMetadataHandle metadata) {
  auto call = MakeCallPair(std::move(metadata), call_handler_.event_engine(),
                           call_handler_.arena(), nullptr);
  destination_->StartCall(std::move(call.unstarted_handler));
  return std::move(call.initiator);
}
}  // namespace interception_chain_detail

namespace {
class CallStarter final : public UnstartedCallDestination {
 public:
  CallStarter(RefCountedPtr<CallFilters::Stack> stack,
              RefCountedPtr<CallDestination> destination)
      : stack_(std::move(stack)), destination_(std::move(destination)) {}

  void Orphan() {
    stack_.reset();
    destination_.reset();
  }

  void StartCall(UnstartedCallHandler unstarted_call_handler) override {
    destination_->HandleCall(unstarted_call_handler.StartCall(stack_));
  }

 private:
  RefCountedPtr<CallFilters::Stack> stack_;
  RefCountedPtr<CallDestination> destination_;
};

class TerminalInterceptor final : public UnstartedCallDestination {
 public:
  explicit TerminalInterceptor(
      RefCountedPtr<CallFilters::Stack> stack,
      RefCountedPtr<UnstartedCallDestination> destination)
      : stack_(std::move(stack)), destination_(std::move(destination)) {}

  void Orphan() {
    stack_.reset();
    destination_.reset();
  }

  void StartCall(UnstartedCallHandler unstarted_call_handler) override {
    unstarted_call_handler.SpawnGuarded(
        "start_call",
        Map(interception_chain_detail::HijackCall(
                std::move(unstarted_call_handler), destination_, stack_),
            [](ValueOrFailure<interception_chain_detail::HijackedCall>
                   hijacked_call) -> StatusFlag {
              if (!hijacked_call.ok()) return Failure{};
              ForwardCall(hijacked_call.value().original_call_handler(),
                          hijacked_call.value().MakeLastCall());
              return Success{};
            }));
  }

 private:
  RefCountedPtr<CallFilters::Stack> stack_;
  RefCountedPtr<UnstartedCallDestination> destination_;
};
}  // namespace

///////////////////////////////////////////////////////////////////////////////
// InterceptionChain::Builder

void InterceptionChainBuilder::AddInterceptor(
    absl::StatusOr<RefCountedPtr<Interceptor>> maybe_interceptor) {
  if (!status_.ok()) return;
  if (!maybe_interceptor.ok()) {
    status_ = maybe_interceptor.status();
    return;
  }
  auto interceptor = std::move(maybe_interceptor.value());
  interceptor->filter_stack_ = MakeFilterStack();
  if (top_interceptor_ == nullptr) {
    top_interceptor_ = std::move(interceptor);
  } else {
    Interceptor* previous = top_interceptor_.get();
    while (previous->wrapped_destination_ != nullptr) {
      previous = down_cast<Interceptor*>(previous->wrapped_destination_.get());
    }
    previous->wrapped_destination_ = std::move(interceptor);
  }
}

absl::StatusOr<RefCountedPtr<UnstartedCallDestination>>
InterceptionChainBuilder::Build(FinalDestination final_destination) {
  if (!status_.ok()) return status_;
  // Build the final UnstartedCallDestination in the chain - what we do here
  // depends on both the type of the final destination and the filters we have
  // that haven't been captured into an Interceptor yet.
  absl::StatusOr<RefCountedPtr<UnstartedCallDestination>> terminator = Match(
      final_destination,
      [this](RefCountedPtr<UnstartedCallDestination> final_destination)
          -> absl::StatusOr<RefCountedPtr<UnstartedCallDestination>> {
        if (stack_builder_.has_value()) {
          // TODO(ctiller): consider interjecting a hijacker here
          return MakeRefCounted<TerminalInterceptor>(MakeFilterStack(),
                                                     final_destination);
        }
        return final_destination;
      },
      [this](RefCountedPtr<CallDestination> final_destination)
          -> absl::StatusOr<RefCountedPtr<UnstartedCallDestination>> {
        return MakeRefCounted<CallStarter>(MakeFilterStack(),
                                           std::move(final_destination));
      });
  if (!terminator.ok()) return terminator.status();
  // Now append the terminator to the interceptor chain.
  if (top_interceptor_ == nullptr) {
    return std::move(terminator.value());
  }
  Interceptor* previous = top_interceptor_.get();
  while (previous->wrapped_destination_ != nullptr) {
    previous = down_cast<Interceptor*>(previous->wrapped_destination_.get());
  }
  previous->wrapped_destination_ = std::move(terminator.value());
  return std::move(top_interceptor_);
}

}  // namespace grpc_core
