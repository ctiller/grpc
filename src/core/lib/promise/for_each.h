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

#ifndef GRPC_CORE_LIB_PROMISE_FOR_EACH_H
#define GRPC_CORE_LIB_PROMISE_FOR_EACH_H

#include "absl/status/status.h"
#include "absl/types/variant.h"
#include "src/core/lib/promise/detail/promise_factory.h"
#include "src/core/lib/promise/poll.h"

namespace grpc_core {

namespace for_each_detail {

template <typename Reader, typename CallPoll>
Poll<absl::Status> FinishIteration(absl::Status* r, Reader* reader,
                                   CallPoll call_poll) {
  if (r->ok()) {
    auto next = reader->Next();
    return call_poll(next);
  }
  return std::move(*r);
}

template <typename T>
struct Done;
template <>
struct Done<absl::Status> {
  static absl::Status Make() { return absl::OkStatus(); }
};

template <typename Reader, typename Action>
class ForEach {
 private:
  using ReaderNext = decltype(std::declval<Reader>().Next());
  using ReaderResult =
      typename decltype(std::declval<ReaderNext>()())::Type::value_type;
  using ActionFactory = promise_detail::PromiseFactory<ReaderResult, Action>;
  using ActionPromise = typename ActionFactory::Promise;

 public:
  using Result = typename decltype(std::declval<ActionPromise>()())::Type;
  ForEach(Reader reader, Action action)
      : reader_(std::move(reader)),
        action_factory_(std::move(action)),
        state_(reader_.Next()) {}

  ForEach(const ForEach&) = delete;
  ForEach& operator=(const ForEach&) = delete;
  ForEach(ForEach&&) noexcept = default;
  ForEach& operator=(ForEach&&) noexcept = default;

  Poll<Result> operator()() {
    return absl::visit(CallPoll<false>{this}, state_);
  }

 private:
  Reader reader_;
  ActionFactory action_factory_;
  absl::variant<ReaderNext, ActionPromise> state_;

  template <bool kSetState>
  struct CallPoll {
    ForEach* const self;

    Poll<Result> operator()(ReaderNext& reader_next) {
      auto r = reader_next();
      if (auto* p = r.get_ready()) {
        if (p->has_value()) {
          auto action = self->action_factory_.Repeated(std::move(**p));
          return CallPoll<true>{self}(action);
        } else {
          return ready(Done<Result>::Make());
        }
      }
      if (kSetState) {
        self->state_.template emplace<ReaderNext>(std::move(reader_next));
      }
      return kPending;
    }

    Poll<Result> operator()(ActionPromise& promise) {
      auto r = promise();
      if (auto* p = r.get_ready()) {
        return FinishIteration(p, &self->reader_, CallPoll<true>{self});
      }
      if (kSetState) {
        self->state_.template emplace<ActionPromise>(std::move(promise));
      }
      return kPending;
    }
  };
};

}  // namespace for_each_detail

/// For each item acquired by calling Reader::Next, run the promise Action.
template <typename Reader, typename Action>
for_each_detail::ForEach<Reader, Action> ForEach(Reader reader, Action action) {
  return for_each_detail::ForEach<Reader, Action>(std::move(reader),
                                                  std::move(action));
}

}  // namespace grpc_core

#endif  // GRPC_CORE_LIB_PROMISE_JOIN_H
