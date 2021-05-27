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

#ifndef GRPC_CORE_LIB_PROMISE_PROMISE_H
#define GRPC_CORE_LIB_PROMISE_PROMISE_H

#include <functional>
#include "src/core/lib/promise/poll.h"

namespace grpc_core {

// A Promise is any functor that takes no arguments and returns Poll<T>.
// Most of the time we just pass around the functor, but occasionally
// it pays to have a type erased variant, which we define here.
template <typename T>
using Promise = std::function<Poll<T>()>;

// Helper to execute a promise immediately and return either the result or
// nothing.
template <typename Promise>
auto NowOrNever(Promise promise)
    -> absl::optional<typename decltype(promise())::Type> {
  auto r = promise();
  if (auto* p = r.get_ready()) {
    return std::move(*p);
  }
  return {};
}

// A promise that never completes.
template <typename T>
struct Never {
  Poll<T> operator()() { return kPending; }
};

namespace promise_detail {
// A promise that immediately completes.
template <typename T>
class Immediate {
 public:
  explicit Immediate(T value) : value_(std::move(value)) {}

  Poll<T> operator()() { return ready(std::move(value_)); }

 private:
  T value_;
};
}  // namespace promise_detail

template <typename T>
promise_detail::Immediate<T> Immediate(T value) {
  return promise_detail::Immediate<T>(std::move(value));
}

}  // namespace grpc_core

#endif  // GRPC_CORE_LIB_PROMISE_PROMISE_H
