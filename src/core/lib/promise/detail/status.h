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

#ifndef GRPC_CORE_LIB_PROMISE_DETAIL_STATUS_H
#define GRPC_CORE_LIB_PROMISE_DETAIL_STATUS_H

#include <grpc/support/port_platform.h>

#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"

#include "src/core/lib/promise/poll.h"

// Helpers for dealing with absl::Status/StatusOr generically

namespace grpc_core {
namespace promise_detail {

// Convert with a move the input status to an absl::Status.
template <typename T>
absl::Status IntoStatus(absl::StatusOr<T>* status) {
  return std::move(status->status());
}

// Convert with a move the input status to an absl::Status.
inline absl::Status IntoStatus(absl::Status* status) {
  return std::move(*status);
}

// Given a promise that returns a StatusOr<T>, return a promise that returns a
// Status.
template <typename Promise>
class FlattenStatus {
 public:
  explicit FlattenStatus(Promise&& promise) : promise_(std::move(promise)) {}

  Poll<absl::Status> operator()() {
    auto r = promise_();
    if (absl::holds_alternative<Pending>(r)) {
      return Pending{};
    }
    return absl::get<kPollReadyIdx>(r).status();
  }

 private:
  Promise promise_;
};

}  // namespace promise_detail

// Given a promise that returns a StatusOr<T>, return a promise that returns a
// Status.
template <typename Promise>
promise_detail::FlattenStatus<Promise> FlattenStatus(Promise promise) {
  return promise_detail::FlattenStatus<Promise>(std::move(promise));
}

// Return true if the status represented by the argument is ok, false if not.
// By implementing this function for other, non-absl::Status types, those types
// can participate in TrySeq as result types that affect control flow.
inline bool IsStatusOk(const absl::Status& status) { return status.ok(); }

}  // namespace grpc_core

#endif  // GRPC_CORE_LIB_PROMISE_DETAIL_STATUS_H
