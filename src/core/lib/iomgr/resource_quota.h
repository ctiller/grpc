/*
 *
 * Copyright 2016 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef GRPC_CORE_LIB_IOMGR_RESOURCE_QUOTA_H
#define GRPC_CORE_LIB_IOMGR_RESOURCE_QUOTA_H

#include <grpc/support/port_platform.h>

#include <grpc/grpc.h>

#include "src/core/lib/debug/trace.h"
#include "src/core/lib/gprpp/orphanable.h"
#include "src/core/lib/gprpp/ref_counted.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/iomgr/closure.h"
#include "src/core/lib/promise/promise.h"

/** \file Tracks resource usage against a pool.

    The current implementation tracks only memory usage, but in the future
    this may be extended to (for example) threads and file descriptors.

    A grpc_resource_quota represents the pooled resources, and
    grpc_resource_user instances attach to the quota and consume those
    resources. They also offer a vector for reclamation: if we become
    resource constrained, grpc_resource_user instances are asked (in turn) to
    free up whatever they can so that the system as a whole can make progress.

    There are three kinds of reclamation that take place, in order of increasing
    invasiveness:
    - an internal reclamation, where cached resource at the resource user level
      is returned to the quota
    - a benign reclamation phase, whereby resources that are in use but are not
      helping anything make progress are reclaimed
    - a destructive reclamation, whereby resources that are helping something
      make progress may be enacted so that at least one part of the system can
      complete.

    Only one reclamation will be outstanding for a given quota at a given time.
    On each reclamation attempt, the kinds of reclamation are tried in order of
    increasing invasiveness, stopping at the first one that succeeds. Thus, on a
    given reclamation attempt, if internal and benign reclamation both fail, it
    will wind up doing a destructive reclamation. However, the next reclamation
    attempt may then be able to get what it needs via internal or benign
    reclamation, due to resources that may have been freed up by the destructive
    reclamation in the previous attempt.

    Future work will be to expose the current resource pressure so that back
    pressure can be applied to avoid reclamation phases starting.

    Resource users own references to resource quotas, and resource quotas
    maintain lists of users (which users arrange to leave before they are
    destroyed) */

extern grpc_core::TraceFlag grpc_resource_quota_trace;

// TODO(juanlishen): This is a hack. We need to do real accounting instead of
// hard coding.
constexpr size_t GRPC_RESOURCE_QUOTA_CALL_SIZE = 15 * 1024;
constexpr size_t GRPC_RESOURCE_QUOTA_CHANNEL_SIZE = 50 * 1024;
constexpr size_t GRPC_SLICE_ALLOCATOR_MIN_ALLOCATE_SIZE = 256;
constexpr size_t GRPC_SLICE_ALLOCATOR_MAX_ALLOCATE_SIZE = 4 * 1024 * 1024;

namespace grpc_core {

class ResourceQuota;
using ResourceQuotaPtr = RefCountedPtr<ResourceQuota>;

class MemoryUser;
using MemoryUserPtr = OrphanablePtr<MemoryUser>;

class ThreadUser;
using ThreadUserPtr = OrphanablePtr<ThreadUser>;

class ResourceQuota final
    : public RefCounted<ResourceQuota, NonPolymorphicRefCount> {
 public:
  static ResourceQuotaPtr FromChannelArgs(grpc_channel_args* channel_args);

  // Return a number indicating current memory pressure:
  // 0.0 ==> no memory usage
  // 1.0 ==> maximum memory usage
  double memory_pressure() const;
  size_t peek_size() const;

  MemoryUserPtr CreateMemoryUser(absl::string_view name);
  ThreadUserPtr CreateThreadUser(absl::string_view name);

 private:
  ResourceQuota();
};

grpc_resource_quota* ToCPtr(ResourceQuotaPtr p) {
  return reinterpret_cast<grpc_resource_quota*>(p.release());
}

ResourceQuotaPtr FromCPtr(grpc_resource_quota* p) {
  return ResourceQuotaPtr(reinterpret_cast<ResourceQuota*>(p));
}

class ThreadUser final : public InternallyRefCounted<ThreadUser> {
 public:
  // Attempts to get quota from the resource_user to create 'thread_count'
  // number
  // of threads. Returns true if successful (i.e the caller is now free to
  // create 'thread_count' number of threads) or false if quota is not available
  bool AllocateThreads(int thread_count);

  // Releases 'thread_count' worth of quota back to the resource user. The quota
  // should have been previously obtained successfully by calling
  // grpc_resource_user_allocate_threads().
  //
  // Note: There need not be an exact one-to-one correspondence between
  // grpc_resource_user_allocate_threads() and grpc_resource_user_free_threads()
  // calls. The only requirement is that the number of threads allocated should
  // all be eventually released.
  void FreeThreads(int thread_count);

  void Orphan();
};

class MemoryUser final : public InternallyRefCounted<MemoryUser> {
 public:
  // Returns a borrowed reference to the underlying resource quota for this
  // resource user.
  ResourceQuota* resource_quota() const;

  // Allocate a block of memory.
  // Will allocate at least min_size bytes, and at most max_size bytes.
  // Returns immediately with the number of bytes allocated, or 0 if
  // insufficient quota.
  size_t MaybeAlloc(size_t min, size_t max) GRPC_MUST_USE_RESULT;

  // Allocate a block of memory.
  // Will allocate at least min_size bytes, and at most max_size bytes.
  // Returns immediately with the number of bytes allocated.
  size_t AllocImmediately(size_t min, size_t max) GRPC_MUST_USE_RESULT;

  // Allocate a block of memory.
  // Will allocate at least min_size bytes, and at most max_size bytes.
  // Calls allocated with the amount of memory allocated once the request can be
  // fulfilled without overflowing memory limits.
  void AllocAsync(size_t min, size_t max, std::function<void(size_t)> allocated);

  // Release memory back to the quota.
  void Free(size_t size);

  // Allocate memory of type T, immediately.
  template <typename T, typename... Args>
  absl::enable_if_t<std::has_virtual_destructor<T>::value, T*> New(
      Args&&... args) {
    GPR_ASSERT(AllocImmediately(sizeof(Wrapped<T>), sizeof(Wrapped<T>)) ==
               sizeof(Wrapped<T>));
    return new Wrapped<T>(Ref(), std::forward<Args>(args)...);
  }

  void Orphan() final;

 private:
  template <typename T>
  class Wrapped final : public T {
   public:
    template <typename... Args>
    Wrapped(RefCountedPtr<MemoryUser> resource_user, Args&&... args)
        : T(std::forward<Args>(args)...) {}
    ~Wrapped() final { resource_user_->Free(sizeof(*this)); }

   private:
    RefCountedPtr<MemoryUser> resource_user_;
  };
};

}  // namespace grpc_core

#endif /* GRPC_CORE_LIB_IOMGR_RESOURCE_QUOTA_H */
