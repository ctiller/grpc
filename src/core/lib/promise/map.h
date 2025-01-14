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

#ifndef GRPC_SRC_CORE_LIB_PROMISE_MAP_H
#define GRPC_SRC_CORE_LIB_PROMISE_MAP_H

#include <grpc/support/port_platform.h>
#include <stddef.h>

#include <tuple>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "src/core/lib/promise/detail/promise_like.h"
#include "src/core/lib/promise/poll.h"

namespace grpc_core {

// Promise mapping combinator - takes a promise and a transformer function,
// and returns a new promise that transforms its output using said function.
// Promise is the type of promise to poll on, Fn is a function that takes the
// result of Promise and maps it to some new type.
template <typename Promise, typename Fn, typename SfinaeVoid = void>
class Map;

template <typename Promise, typename Fn>
class Map<
    Promise, Fn,
    absl::enable_if_t<!std::is_void<std::invoke_result_t<
        Fn, typename promise_detail::PromiseLike<Promise>::Result>>::value>> {
 public:
  GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION Map(Promise promise, Fn fn)
      : promise_(std::move(promise)), fn_(std::move(fn)) {}

  Map(const Map&) = delete;
  Map& operator=(const Map&) = delete;
  // NOLINTNEXTLINE(performance-noexcept-move-constructor): clang6 bug
  Map(Map&& other) = default;
  // NOLINTNEXTLINE(performance-noexcept-move-constructor): clang6 bug
  Map& operator=(Map&& other) = default;

  using PromiseResult = typename promise_detail::PromiseLike<Promise>::Result;
  using Result = promise_detail::RemoveCVRef<decltype(std::declval<Fn>()(
      std::declval<PromiseResult>()))>;

  GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION Poll<Result> operator()() {
    Poll<PromiseResult> r = promise_();
    if (auto* p = r.value_if_ready()) {
      return fn_(std::move(*p));
    }
    return Pending();
  }

 private:
  promise_detail::PromiseLike<Promise> promise_;
  Fn fn_;
};

template <typename Promise, typename Fn>
class Map<
    Promise, Fn,
    absl::enable_if_t<std::is_void<std::invoke_result_t<
        Fn, typename promise_detail::PromiseLike<Promise>::Result>>::value>> {
 public:
  GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION Map(Promise promise, Fn fn)
      : promise_(std::move(promise)), fn_(std::move(fn)) {}

  Map(const Map&) = delete;
  Map& operator=(const Map&) = delete;
  // NOLINTNEXTLINE(performance-noexcept-move-constructor): clang6 bug
  Map(Map&& other) = default;
  // NOLINTNEXTLINE(performance-noexcept-move-constructor): clang6 bug
  Map& operator=(Map&& other) = default;

  using PromiseResult = typename promise_detail::PromiseLike<Promise>::Result;
  using Result = Empty;

  GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION Poll<Result> operator()() {
    Poll<PromiseResult> r = promise_();
    if (auto* p = r.value_if_ready()) {
      fn_(std::move(*p));
      return Empty{};
    }
    return Pending();
  }

 private:
  promise_detail::PromiseLike<Promise> promise_;
  Fn fn_;
};

template <typename Promise, typename Fn>
Map(Promise, Fn) -> Map<Promise, Fn>;

// Maps a promise to a new promise that returns a tuple of the original result
// and a bool indicating whether there was ever a Pending{} value observed from
// polling.
template <typename Promise>
GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION inline auto CheckDelayed(Promise promise) {
  using P = promise_detail::PromiseLike<Promise>;
  return [delayed = false, promise = P(std::move(promise))]() mutable
             -> Poll<std::tuple<typename P::Result, bool>> {
    auto r = promise();
    if (r.pending()) {
      delayed = true;
      return Pending{};
    }
    return std::make_tuple(std::move(r.value()), delayed);
  };
}

// Callable that takes a tuple and returns one element
template <size_t kElem>
struct JustElem {
  template <typename... A>
  GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION auto operator()(std::tuple<A...>&& t)
      const -> decltype(std::get<kElem>(std::forward<std::tuple<A...>>(t))) {
    return std::get<kElem>(std::forward<std::tuple<A...>>(t));
  }
  template <typename... A>
  GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION auto operator()(
      const std::tuple<A...>& t) const -> decltype(std::get<kElem>(t)) {
    return std::get<kElem>(t);
  }
};

namespace promise_detail {
template <typename Fn>
class MapError {
 public:
  explicit MapError(Fn fn) : fn_(std::move(fn)) {}
  absl::Status operator()(absl::Status status) {
    if (status.ok()) return status;
    return fn_(std::move(status));
  }
  template <typename T>
  absl::StatusOr<T> operator()(absl::StatusOr<T> status) {
    if (status.ok()) return status;
    return fn_(std::move(status.status()));
  }

 private:
  Fn fn_;
};
}  // namespace promise_detail

// Map status->better status in the case of errors
template <typename Promise, typename Fn>
auto MapErrors(Promise promise, Fn fn) {
  return Map(std::move(promise), promise_detail::MapError<Fn>(std::move(fn)));
}

// Simple mapper to add a prefix to the message of an error
template <typename Promise>
auto AddErrorPrefix(absl::string_view prefix, Promise promise) {
  return MapErrors(std::move(promise), [prefix](absl::Status status) {
    absl::Status out(status.code(), absl::StrCat(prefix, status.message()));
    status.ForEachPayload(
        [&out](absl::string_view name, const absl::Cord& value) {
          out.SetPayload(name, value);
        });
    return out;
  });
}

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_PROMISE_MAP_H
