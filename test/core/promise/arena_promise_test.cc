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

#include "src/core/lib/promise/arena_promise.h"

#include <memory>

#include <gtest/gtest.h>

#include "src/core/lib/resource_quota/resource_quota.h"
#include "test/core/promise/test_context.h"

namespace grpc_core {

static auto* g_memory_allocator = new MemoryAllocator(
    ResourceQuota::Default()->memory_quota()->CreateMemoryAllocator("test"));

TEST(ArenaPromiseTest, DefaultInitializationYieldsNoValue) {
  auto arena = MakeScopedArena(1024, g_memory_allocator);
  TestContext<Arena> context(arena.get());
  ArenaPromise<int> p;
  EXPECT_FALSE(p.has_value());
}

TEST(ArenaPromiseTest, AllocatedWorks) {
  auto arena = MakeScopedArena(1024, g_memory_allocator);
  TestContext<Arena> context(arena.get());
  int x = 42;
  ArenaPromise<int> p([x] { return Poll<int>(x); });
  EXPECT_TRUE(p.has_value());
  EXPECT_EQ(p(), Poll<int>(42));
  p = ArenaPromise<int>([] { return Poll<int>(43); });
  EXPECT_EQ(p(), Poll<int>(43));
}

TEST(ArenaPromiseTest, DestructionWorks) {
  auto arena = MakeScopedArena(1024, g_memory_allocator);
  TestContext<Arena> context(arena.get());
  auto x = std::make_shared<int>(42);
  auto p = ArenaPromise<int>([x] { return Poll<int>(*x); });
  ArenaPromise<int> q(std::move(p));
  EXPECT_EQ(q(), Poll<int>(42));
}

TEST(ArenaPromiseTest, MoveAssignmentWorks) {
  auto arena = MakeScopedArena(1024, g_memory_allocator);
  TestContext<Arena> context(arena.get());
  auto x = std::make_shared<int>(42);
  auto p = ArenaPromise<int>([x] { return Poll<int>(*x); });
  p = ArenaPromise<int>();
}

}  // namespace grpc_core

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
