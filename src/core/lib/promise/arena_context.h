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

#ifndef ARENA_CONTEXT_H
#define ARENA_CONTEXT_H

#include "src/core/lib/promise/context.h"
#include "src/core/lib/resource_quota/arena.h"

namespace grpc_core {

template <typename T>
struct ArenaContextType;

namespace promise_detail {

class ArenaContextTraitsCommon {
 public:
  static int Allocate(void (*destructor)(void*)) {
    int index = destructors_->size();
    destructors_->push_back(destructor);
    return index;
  }

  static int ContextCount() { return destructors_->size(); }

 private:
  using Destructor = void (*)(void*);
  static NoDestruct<std::vector<Destructor>> destructors_;
};

template <typename T>
class ArenaContextTraits : public ArenaContextType<T> {
 public:
  static int index() { return index_; }

 private:
  static const int index_;
};

template <typename T>
const int ArenaContextTraits<T>::index_ = ArenaContextTraitsCommon::Allocate(
    [](void* p) { static_cast<T*>(p)->~T(); });

}  // namespace promise_detail

class ArenaContext {
 public:
  explicit ArenaContext(Arena* arena = GetContext<Arena>())
      : contexts_(static_cast<void**>(arena->Alloc(
            sizeof(void*) *
            promise_detail::ArenaContextTraitsCommon::ContextCount()))) {}

  template <typename T>
  T* NewContext() {
    const int index = promise_detail::ArenaContextTraits<T>::index();
    void*& context = contexts_[index];
    if (owned_contexts_ & (1 << index)) {
      static_cast<T*>(context)->~T();
    }
    owned_contexts_ |= (1 << index);
    context = GetContext<Arena>()->New<T>();
  }

  template <typename T>
  void SetUnowned(T* p) {
    const int index = promise_detail::ArenaContextTraits<T>::index();
    void*& context = contexts_[index];
    if (owned_contexts_ & (1 << index)) {
      static_cast<T*>(context)->~T();
      owned_contexts_ &= ~(1 << index);
    }
    context = p;
  }

  template <typename T>
  T* Get() {
    const int index = promise_detail::ArenaContextTraits<T>::index();
    void*& context = contexts_[index];
    return static_cast<T*>(context);
  }

 private:
  uint32_t owned_contexts_ = 0;
  void** contexts_;
};

template <>
struct ContextType<ArenaContext> {};

namespace promise_detail {
template <typename T>
class Context<T, absl::void_t<decltype(ArenaContextType<T>())>> {
 public:
  static T* get() { return GetContext<ArenaContext>()->Get<T>(); }
};
}  // namespace promise_detail

}  // namespace grpc_core

#endif
