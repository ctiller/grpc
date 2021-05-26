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

#ifndef GRPC_CORE_LIB_PROMISE_CONTEXT_H
#define GRPC_CORE_LIB_PROMISE_CONTEXT_H

#include "src/core/lib/promise/poll.h"

namespace grpc_core {

// To avoid accidentally creating context types, we require an explicit
// specialization of this template per context type. The specialization need
// not contain any members.
template <typename T>
struct ContextType;

namespace context_detail {

template <typename T>
class Context : public ContextType<T> {
 public:
  explicit Context(T* p) : old_(current_) { current_ = p; }
  ~Context() { current_ = old_; }
  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  static T* get() { return current_; }

 private:
  static thread_local T* current_;
  T* const old_;
};

template <typename T>
thread_local T* Context<T>::current_;

template <typename T, typename F>
class WithContext {
 public:
  WithContext(F f, T* context) : context_(context), f_(std::move(f)) {}

  decltype(std::declval<F>()()) operator()() {
    Context<T> ctx(context_);
    return f_();
  }

 private:
  T* context_;
  F f_;
};

}  // namespace context_detail

// Retrieve the current value of a context.
template <typename T>
T* GetContext() {
  return context_detail::Context<T>::get();
}

// Given a promise and a context, return a promise that has that context set.
template <typename T, typename F>
context_detail::WithContext<T, F> WithContext(F f, T* context) {
  return context_detail::WithContext<T, F>(f, context);
}

}  // namespace grpc_core

#endif  // GRPC_CORE_LIB_PROMISE_JOIN_H
