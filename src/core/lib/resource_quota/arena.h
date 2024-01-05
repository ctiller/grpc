//
//
// Copyright 2017 gRPC authors.
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
//
//

// \file Arena based allocator
// Allows very fast allocation of memory, but that memory cannot be freed until
// the arena as a whole is freed
// Tracks the total memory allocated against it, so that future arenas can
// pre-allocate the right amount of memory

#ifndef GRPC_SRC_CORE_LIB_RESOURCE_QUOTA_ARENA_H
#define GRPC_SRC_CORE_LIB_RESOURCE_QUOTA_ARENA_H

#include <grpc/support/port_platform.h>

#include <stddef.h>

#include <atomic>
#include <iosfwd>
#include <memory>
#include <utility>

#include <grpc/event_engine/memory_allocator.h>

#include "src/core/lib/gpr/alloc.h"
#include "src/core/lib/gprpp/construct_destruct.h"
#include "src/core/lib/promise/context.h"
#include "src/core/lib/resource_quota/memory_quota.h"

#define GRPC_ARENA_POOLED_ALLOCATIONS_USE_MALLOC
// #define GRPC_ARENA_TRACE_POOLED_ALLOCATIONS

namespace grpc_core {

namespace arena_detail {

// Arenas can carry some context objects.
template <typename T>
struct ContextType;

template <typename T, typename A, typename B>
struct IfArray {
  using Result = A;
};

template <typename T, typename A, typename B>
struct IfArray<T[], A, B> {
  using Result = B;
};

}  // namespace arena_detail

class Arena {
 public:
  // Create an arena, with \a initial_size bytes in the first allocated buffer.
  static Arena* Create(size_t initial_size, MemoryAllocator* memory_allocator);

  // Create an arena, with \a initial_size bytes in the first allocated buffer,
  // and return both a void pointer to the returned arena and a void* with the
  // first allocation.
  static std::pair<Arena*, void*> CreateWithAlloc(
      size_t initial_size, size_t alloc_size,
      MemoryAllocator* memory_allocator);

  // Destroy all `ManagedNew` allocated objects.
  // Allows safe destruction of these objects even if they need context held by
  // the arena.
  // Idempotent.
  // TODO(ctiller): eliminate ManagedNew.
  void DestroyManagedNewObjects();

  // Destroy an arena.
  void Destroy();

  // Return the total amount of memory allocated by this arena.
  size_t TotalUsedBytes() const {
    return total_used_.load(std::memory_order_relaxed);
  }

  // Allocate \a size bytes from the arena.
  void* Alloc(size_t size) {
    static constexpr size_t base_size =
        GPR_ROUND_UP_TO_ALIGNMENT_SIZE(sizeof(Arena));
    size = GPR_ROUND_UP_TO_ALIGNMENT_SIZE(size);
    size_t begin = total_used_.fetch_add(size, std::memory_order_relaxed);
    if (begin + size <= initial_zone_size_) {
      return reinterpret_cast<char*>(this) + base_size + begin;
    } else {
      return AllocZone(size);
    }
  }

  // TODO(roth): We currently assume that all callers need alignment of 16
  // bytes, which may be wrong in some cases. When we have time, we should
  // change this to instead use the alignment of the type being allocated by
  // this method.
  template <typename T, typename... Args>
  T* New(Args&&... args) {
    T* t = static_cast<T*>(Alloc(sizeof(T)));
    Construct(t, std::forward<Args>(args)...);
    return t;
  }

  // Like New, but has the arena call p->~T() at arena destruction time.
  template <typename T, typename... Args>
  T* ManagedNew(Args&&... args) {
    auto* p = New<ManagedNewImpl<T>>(std::forward<Args>(args)...);
    p->Link(&managed_new_head_);
    return &p->t;
  }
  class PooledDeleter {
   public:
    PooledDeleter() = default;
    explicit PooledDeleter(std::nullptr_t) : delete_(false) {}
    template <typename T>
    void operator()(T* p) {
      // TODO(ctiller): promise based filter hijacks ownership of some pointers
      // to make them appear as PoolPtr without really transferring ownership,
      // by setting the arena to nullptr.
      // This is a transitional hack and should be removed once promise based
      // filter is removed.
      if (delete_) delete p;
    }

    bool has_freelist() const { return delete_; }

   private:
    bool delete_ = true;
  };

  class ArrayPooledDeleter {
   public:
    ArrayPooledDeleter() = default;
    explicit ArrayPooledDeleter(std::nullptr_t) : delete_(false) {}
    template <typename T>
    void operator()(T* p) {
      // TODO(ctiller): promise based filter hijacks ownership of some pointers
      // to make them appear as PoolPtr without really transferring ownership,
      // by setting the arena to nullptr.
      // This is a transitional hack and should be removed once promise based
      // filter is removed.
      if (delete_) delete[] p;
    }

    bool has_freelist() const { return delete_; }

   private:
    bool delete_ = true;
  };

  template <typename T>
  using PoolPtr =
      std::unique_ptr<T, typename arena_detail::IfArray<
                             T, PooledDeleter, ArrayPooledDeleter>::Result>;

  // Make a unique_ptr to T that is allocated from the arena.
  // When the pointer is released, the memory may be reused for other
  // MakePooled(.*) calls.
  // CAUTION: The amount of memory allocated is rounded up to the nearest
  //          value in Arena::PoolSizes, and so this may pessimize total
  //          arena size.
  template <typename T, typename... Args>
  PoolPtr<T> MakePooled(Args&&... args) {
    return PoolPtr<T>(new T(std::forward<Args>(args)...), PooledDeleter());
  }

  // Make a unique_ptr to an array of T that is allocated from the arena.
  // When the pointer is released, the memory may be reused for other
  // MakePooled(.*) calls.
  // One can use MakePooledArray<char> to allocate a buffer of bytes.
  // CAUTION: The amount of memory allocated is rounded up to the nearest
  //          value in Arena::PoolSizes, and so this may pessimize total
  //          arena size.
  template <typename T>
  PoolPtr<T[]> MakePooledArray(size_t n) {
    return PoolPtr<T[]>(new T[n], ArrayPooledDeleter());
  }

  // Like MakePooled, but with manual memory management.
  // The caller is responsible for calling DeletePooled() on the returned
  // pointer, and expected to call it with the same type T as was passed to this
  // function (else the free list returned to the arena will be corrupted).
  template <typename T, typename... Args>
  T* NewPooled(Args&&... args) {
    return new T(std::forward<Args>(args)...);
  }

  template <typename T>
  void DeletePooled(T* p) {
    delete p;
  }

 private:
  struct Zone {
    Zone* prev;
  };

  struct ManagedNewObject {
    ManagedNewObject* next = nullptr;
    void Link(std::atomic<ManagedNewObject*>* head);
    virtual ~ManagedNewObject() = default;
  };

  template <typename T>
  struct ManagedNewImpl : public ManagedNewObject {
    T t;
    template <typename... Args>
    explicit ManagedNewImpl(Args&&... args) : t(std::forward<Args>(args)...) {}
  };

  // Initialize an arena.
  // Parameters:
  //   initial_size: The initial size of the whole arena in bytes. These bytes
  //   are contained within 'zone 0'. If the arena user ends up requiring more
  //   memory than the arena contains in zone 0, subsequent zones are allocated
  //   on demand and maintained in a tail-linked list.
  //
  //   initial_alloc: Optionally, construct the arena as though a call to
  //   Alloc() had already been made for initial_alloc bytes. This provides a
  //   quick optimization (avoiding an atomic fetch-add) for the common case
  //   where we wish to create an arena and then perform an immediate
  //   allocation.
  explicit Arena(size_t initial_size, size_t initial_alloc,
                 MemoryAllocator* memory_allocator)
      : total_used_(GPR_ROUND_UP_TO_ALIGNMENT_SIZE(initial_alloc)),
        initial_zone_size_(initial_size),
        memory_allocator_(memory_allocator) {}

  ~Arena();

  void* AllocZone(size_t size);

  // Keep track of the total used size. We use this in our call sizing
  // hysteresis.
  std::atomic<size_t> total_used_{0};
  std::atomic<size_t> total_allocated_{0};
  const size_t initial_zone_size_;
  // If the initial arena allocation wasn't enough, we allocate additional zones
  // in a reverse linked list. Each additional zone consists of (1) a pointer to
  // the zone added before this zone (null if this is the first additional zone)
  // and (2) the allocated memory. The arena itself maintains a pointer to the
  // last zone; the zone list is reverse-walked during arena destruction only.
  std::atomic<Zone*> last_zone_{nullptr};
  std::atomic<ManagedNewObject*> managed_new_head_{nullptr};
  // The backing memory quota
  MemoryAllocator* const memory_allocator_;
};

// Smart pointer for arenas when the final size is not required.
struct ScopedArenaDeleter {
  void operator()(Arena* arena) { arena->Destroy(); }
};
using ScopedArenaPtr = std::unique_ptr<Arena, ScopedArenaDeleter>;
inline ScopedArenaPtr MakeScopedArena(size_t initial_size,
                                      MemoryAllocator* memory_allocator) {
  return ScopedArenaPtr(Arena::Create(initial_size, memory_allocator));
}

// Arenas form a context for activities
template <>
struct ContextType<Arena> {};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_RESOURCE_QUOTA_ARENA_H
