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

#include "src/core/lib/promise/if.h"
#include <gtest/gtest.h>

namespace grpc_core {

TEST(IfTest, ChooseTrue) {
  EXPECT_EQ(If([]() { return ready(true); }, []() { return ready(1); },
               []() { return ready(2); })()
                .take(),
            1);
}

TEST(IfTest, ChooseFalse) {
  EXPECT_EQ(If([]() { return ready(false); }, []() { return ready(1); },
               []() { return ready(2); })()
                .take(),
            2);
}

TEST(IfTest, FactoryChooseTrue) {
  EXPECT_EQ(If([] { return [] { return ready(true); }; },
               [] { return [] { return ready(1); }; },
               [] { return [] { return ready(2); }; })()
                .take(),
            1);
}

TEST(IfTest, FactoryChooseFalse) {
  EXPECT_EQ(If([] { return [] { return ready(false); }; },
               [] { return [] { return ready(1); }; },
               [] { return [] { return ready(2); }; })()
                .take(),
            2);
}

TEST(IfTest, ChooseSuccesfulTrue) {
  EXPECT_EQ(If([]() { return ready(absl::StatusOr<bool>(true)); },
               []() { return ready(absl::StatusOr<int>(1)); },
               []() { return ready(absl::StatusOr<int>(2)); })()
                .take(),
            absl::StatusOr<int>(1));
}

TEST(IfTest, ChooseSuccesfulFalse) {
  EXPECT_EQ(If([]() { return ready(absl::StatusOr<bool>(false)); },
               []() { return ready(absl::StatusOr<int>(1)); },
               []() { return ready(absl::StatusOr<int>(2)); })()
                .take(),
            absl::StatusOr<int>(2));
}

TEST(IfTest, FactoryChooseSuccesfulTrue) {
  EXPECT_EQ(If([] { return [] { return ready(absl::StatusOr<bool>(true)); }; },
               [] { return [] { return ready(absl::StatusOr<int>(1)); }; },
               [] { return [] { return ready(absl::StatusOr<int>(2)); }; })()
                .take(),
            absl::StatusOr<int>(1));
}

TEST(IfTest, FactoryChooseSuccesfulFalse) {
  EXPECT_EQ(If([] { return [] { return ready(absl::StatusOr<bool>(false)); }; },
               [] { return [] { return ready(absl::StatusOr<int>(1)); }; },
               [] { return [] { return ready(absl::StatusOr<int>(2)); }; })()
                .take(),
            absl::StatusOr<int>(2));
}

TEST(IfTest, ChooseFailure) {
  EXPECT_EQ(If([]() { return ready(absl::StatusOr<bool>()); },
               []() { return ready(absl::StatusOr<int>(1)); },
               []() { return ready(absl::StatusOr<int>(2)); })()
                .take(),
            absl::StatusOr<int>());
}

TEST(IfTest, FactoryChooseFailure) {
  EXPECT_EQ(If([] { return [] { return ready(absl::StatusOr<bool>()); }; },
               [] { return [] { return ready(absl::StatusOr<int>(1)); }; },
               [] { return [] { return ready(absl::StatusOr<int>(2)); }; })()
                .take(),
            absl::StatusOr<int>());
}

}  // namespace grpc_core

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
