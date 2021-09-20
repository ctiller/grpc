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

#include "src/core/lib/resource_quota/memory_quota.h"

#include <gtest/gtest.h>

#include "absl/synchronization/notification.h"

namespace grpc_core {
namespace testing {

//
// Helpers
//

template <size_t kSize>
struct Sized {
  char blah[kSize];
  virtual ~Sized() {}
};

//
// MemoryRequestTest
//

TEST(MemoryRequestTest, ConversionFromSize) {
  MemoryRequest request = 3;
  EXPECT_EQ(request.min(), 3);
  EXPECT_EQ(request.max(), 3);
}

TEST(MemoryRequestTest, MinMax) {
  MemoryRequest request(3, 7);
  EXPECT_EQ(request.min(), 3);
  EXPECT_EQ(request.max(), 7);
}

//
// MemoryQuotaTest
//

TEST(MemoryQuotaTest, NoOp) {
  RefCountedPtr<MemoryQuota> memory_quota = MakeRefCounted<MemoryQuota>();
}

TEST(MemoryQuotaTest, CreateAllocatorNoOp) {
  RefCountedPtr<MemoryQuota> memory_quota = MakeRefCounted<MemoryQuota>();
  auto memory_allocator = memory_quota->MakeMemoryAllocator();
}

TEST(MemoryQuotaTest, CreateObjectFromAllocator) {
  RefCountedPtr<MemoryQuota> memory_quota = MakeRefCounted<MemoryQuota>();
  auto memory_allocator = memory_quota->MakeMemoryAllocator();
  auto object = memory_allocator->MakeUnique<Sized<4096>>();
}

TEST(MemoryQuotaTest, CreateSomeObjectsAndExpectReclamation) {
  RefCountedPtr<MemoryQuota> memory_quota = MakeRefCounted<MemoryQuota>();
  memory_quota->SetSize(4096);
  auto memory_allocator = memory_quota->MakeMemoryAllocator();
  auto object = memory_allocator->MakeUnique<Sized<2048>>();

  absl::Notification notification;
  memory_allocator->PostReclaimer(ReclamationPass::kDestructive,
                                  [&notification, &object](ReclamationSweep) {
                                    object.reset();
                                    notification.Notify();
                                  });
  auto object2 = memory_allocator->MakeUnique<Sized<2048>>();
  notification.WaitForNotification();
  EXPECT_EQ(object.get(), nullptr);

  absl::Notification notification2;
  memory_allocator->PostReclaimer(ReclamationPass::kDestructive,
                                  [&notification2, &object2](ReclamationSweep) {
                                    object2.reset();
                                    notification2.Notify();
                                  });
  auto object3 = memory_allocator->MakeUnique<Sized<2048>>();
  notification2.WaitForNotification();
  EXPECT_EQ(object2.get(), nullptr);
}

TEST(MemoryQuotaTest, BasicRebind) {
  RefCountedPtr<MemoryQuota> memory_quota = MakeRefCounted<MemoryQuota>();
  memory_quota->SetSize(4096);
  RefCountedPtr<MemoryQuota> memory_quota2 = MakeRefCounted<MemoryQuota>();
  memory_quota2->SetSize(4096);

  auto memory_allocator = memory_quota2->MakeMemoryAllocator();
  auto object = memory_allocator->MakeUnique<Sized<2048>>();

  memory_allocator->Rebind(memory_quota);
  auto memory_allocator2 = memory_quota2->MakeMemoryAllocator();

  memory_allocator2->PostReclaimer(ReclamationPass::kDestructive,
                                   [](ReclamationSweep) {
                                     // Taken memory should be reassigned to
                                     // memory_quota, so this should never be
                                     // reached.
                                     abort();
                                   });

  absl::Notification notification;
  memory_allocator->PostReclaimer(ReclamationPass::kDestructive,
                                  [&object, &notification](ReclamationSweep) {
                                    // The new memory allocator should reclaim
                                    // the object allocated against the previous
                                    // quota because that's now part of this
                                    // quota.
                                    object.reset();
                                    notification.Notify();
                                  });

  auto object2 = memory_allocator->MakeUnique<Sized<2048>>();
  notification.WaitForNotification();
  EXPECT_EQ(object.get(), nullptr);
}

TEST(MemoryQuotaTest, ReserveRangeNoPressure) {
  RefCountedPtr<MemoryQuota> memory_quota = MakeRefCounted<MemoryQuota>();
  auto memory_allocator = memory_quota->MakeMemoryAllocator();
  size_t total = 0;
  for (int i = 0; i < 10000; i++) {
    auto n = memory_allocator->Reserve(MemoryRequest(100, 40000));
    EXPECT_EQ(n, 40000);
    total += n;
  }
  memory_allocator->Release(total);
}

}  // namespace testing
}  // namespace grpc_core

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  gpr_log_verbosity_init();
  return RUN_ALL_TESTS();
}
