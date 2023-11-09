// Copyright 2023 gRPC authors.
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

#include "src/core/lib/channel/promise_based_filter.h"

namespace grpc_core {

struct NoOpInterception {};

namespace filter_detail {

template <typename T>
auto MakeCallBody(T*, typename T::Call*, CallArgs call_args,
                  NextPromiseFactory next_promise_factory,
                  const NoOpInterception*, const NoOpInterception*) {
  return next_promise_factory(std::move(call_args));
}

template <typename T>
std::enable_if_t<std::is_empty<typename T::Call>::value, typename T::Call*>
AllocateCall() {
  static char x;
  return reinterpret_cast<T*>(&x);
}

template <typename T>
std::enable_if_t<!std::is_empty<typename T::Call>::value, typename T::Call*>
AllocateCall() {
  return GetContext<Arena>()->ManagedNew<T>();
}

template <typename T, typename Pipe>
void Intercept(T*, typename T::Call*, Pipe*, const NoOpInterception*) {}

template <typename T, typename Pipe, typename V>
void Intercept(T*, typename T::Call* call, Pipe* pipe,
               void (T::Call::*interceptor)(V&)) {
  return pipe->InterceptAndMap([call](typename Pipe::ValueType v) {
    call->interceptor(*v);
    return v;
  });
}

}  // namespace filter_detail

template <typename Derived>
class FilterImplementation : public ChannelFilter {
 public:
  // Legacy MakeCallPromise interop
  ArenaPromise<ServerMetadataHandle> MakeCallPromise(
      CallArgs call_args, NextPromiseFactory next_promise_factory) override {
    auto* body = filter_detail::AllocateCall<Derived>();
    filter_detail::Intercept(this, body, &call_args.client_to_server_messages,
                             &Derived::OnClientToServerMessage);
    filter_detail::Intercept(this, body, &call_args.server_to_client_messages,
                             &Derived::OnServerToClientMessage);
    return filter_detail::MakeCallBody(
        static_cast<Derived>(this), body, std::move(call_args),
        std::move(next_promise_factory),
        &Derived::CallType::OnClientInitialMetadata,
        &Derived::CallType::OnServerTrailingMetadata);
  }
};

}  // namespace grpc_core
