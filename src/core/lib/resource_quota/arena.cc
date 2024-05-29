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

#include "src/core/lib/resource_quota/arena.h"

#include <atomic>
#include <new>

#include <grpc/support/alloc.h>
#include <grpc/support/port_platform.h>

#include "src/core/lib/resource_quota/resource_quota.h"
#include "src/core/util/alloc.h"

namespace {

void* ArenaStorage(size_t initial_size) {
  static constexpr size_t base_size =
      GPR_ROUND_UP_TO_ALIGNMENT_SIZE(sizeof(grpc_core::Arena));
  initial_size = GPR_ROUND_UP_TO_ALIGNMENT_SIZE(initial_size);
  size_t alloc_size = base_size + initial_size;
  static constexpr size_t alignment =
      (GPR_CACHELINE_SIZE > GPR_MAX_ALIGNMENT &&
       GPR_CACHELINE_SIZE % GPR_MAX_ALIGNMENT == 0)
          ? GPR_CACHELINE_SIZE
          : GPR_MAX_ALIGNMENT;
  return gpr_malloc_aligned(alloc_size, alignment);
}

}  // namespace

namespace grpc_core {

Arena::~Arena() {
  Zone* z = last_zone_;
  while (z) {
    Zone* prev_z = z->prev;
    Destruct(z);
    gpr_free_aligned(z);
    z = prev_z;
  }
#ifdef GRPC_ARENA_TRACE_POOLED_ALLOCATIONS
  gpr_log(GPR_ERROR, "DESTRUCT_ARENA %p", this);
#endif
}

RefCountedPtr<Arena> Arena::Create(size_t initial_size,
                                   RefCountedPtr<ArenaFactory> arena_factory) {
  return RefCountedPtr<Arena>(new (ArenaStorage(initial_size)) Arena(
      initial_size, 0, std::move(arena_factory)));
}

Arena::Arena(size_t initial_size, size_t initial_alloc,
             RefCountedPtr<ArenaFactory> arena_factory)
    : total_used_(GPR_ROUND_UP_TO_ALIGNMENT_SIZE(initial_alloc)),
      initial_zone_size_(initial_size),
      arena_factory_(std::move(arena_factory)) {
  arena_factory_->allocator().Reserve(
      GPR_ROUND_UP_TO_ALIGNMENT_SIZE(initial_alloc));
}

void Arena::DestroyManagedNewObjects() {
  ManagedNewObject* p;
  // Outer loop: clear the managed new object list.
  // We do this repeatedly in case a destructor ends up allocating something.
  while ((p = managed_new_head_.exchange(nullptr, std::memory_order_relaxed)) !=
         nullptr) {
    // Inner loop: destruct a batch of objects.
    while (p != nullptr) {
      Destruct(std::exchange(p, p->next));
    }
  }
}

void Arena::Destroy() {
  DestroyManagedNewObjects();
  arena_factory_->allocator().Release(
      total_allocated_.load(std::memory_order_relaxed));
  this->~Arena();
  gpr_free_aligned(this);
}

void* Arena::AllocZone(size_t size) {
  // If the allocation isn't able to end in the initial zone, create a new
  // zone for this allocation, and any unused space in the initial zone is
  // wasted. This overflowing and wasting is uncommon because of our arena
  // sizing hysteresis (that is, most calls should have a large enough initial
  // zone and will not need to grow the arena).
  static constexpr size_t zone_base_size =
      GPR_ROUND_UP_TO_ALIGNMENT_SIZE(sizeof(Zone));
  size_t alloc_size = zone_base_size + size;
  arena_factory_->allocator().Reserve(alloc_size);
  total_allocated_.fetch_add(alloc_size, std::memory_order_relaxed);
  Zone* z = new (gpr_malloc_aligned(alloc_size, GPR_MAX_ALIGNMENT)) Zone();
  auto* prev = last_zone_.load(std::memory_order_relaxed);
  do {
    z->prev = prev;
  } while (!last_zone_.compare_exchange_weak(prev, z, std::memory_order_relaxed,
                                             std::memory_order_relaxed));
  return reinterpret_cast<char*>(z) + zone_base_size;
}

void Arena::ManagedNewObject::Link(std::atomic<ManagedNewObject*>* head) {
  next = head->load(std::memory_order_relaxed);
  while (!head->compare_exchange_weak(next, this, std::memory_order_acq_rel,
                                      std::memory_order_relaxed)) {
  }
}

RefCountedPtr<ArenaFactory> SimpleArenaAllocator(size_t initial_size) {
  class Allocator : public ArenaFactory {
   public:
    explicit Allocator(size_t initial_size)
        : ArenaFactory(
              ResourceQuota::Default()->memory_quota()->CreateMemoryAllocator(
                  "simple-arena-allocator")),
          initial_size_(initial_size) {}

    RefCountedPtr<Arena> MakeArena() override {
      return Arena::Create(initial_size_, Ref());
    }

    void FinalizeArena(Arena* arena) override {
      // No-op.
    }

   private:
    size_t initial_size_;
  };
  return MakeRefCounted<Allocator>(initial_size);
}

}  // namespace grpc_core
