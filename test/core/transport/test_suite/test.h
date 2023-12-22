// Copyright 2023 gRPC authors.
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

#ifndef TESTSUITETEST_H
#define TESTSUITETEST_H

#include <memory>

#include "absl/functional/any_invocable.h"
#include "absl/strings/string_view.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "src/core/lib/iomgr/timer_manager.h"
#include "src/core/lib/resource_quota/resource_quota.h"
#include "test/core/event_engine/fuzzing_event_engine/fuzzing_event_engine.h"
#include "test/core/event_engine/fuzzing_event_engine/fuzzing_event_engine.pb.h"
#include "test/core/transport/test_suite/fixture.h"

namespace grpc_core {

class MockAcceptor : public ServerTransport::Acceptor {
 public:
  MOCK_METHOD(Arena*, CreateArena, (), (override));
  MOCK_METHOD(absl::StatusOr<CallInitiator>, CreateCall,
              (ClientMetadata & client_initial_metadata, Arena* arena),
              (override));
};

class TransportTest : public ::testing::Test {
 protected:
  TransportTest(std::unique_ptr<TransportFixture> fixture)
      : fixture_(std::move(fixture)) {}

  void RunTest();

  void SetServerAcceptor();
  CallInitiator CreateCall();

  MockAcceptor& acceptor() { return acceptor_; }

  CallHandler TickUntilServerCall() {
    auto* arena = Arena::Create(1024, &allocator_);
    EXPECT_CALL(acceptor_, CreateArena()).WillOnce(::testing::Return(arena));
    absl::optional<CallHandler> call_handler;
    EXPECT_CALL(acceptor_, CreateCall(::testing::_, arena))
        .WillOnce(::testing::WithArgs<0, 1>(
            [this, &call_handler](ClientMetadata& client_metadata,
                                  Arena* arena) {
              auto call = MakeCall(event_engine_.get(), arena);
              call_handler.emplace(std::move(call.handler));
              return call.initiator;
            }));
    while (!call_handler.has_value()) {
      event_engine_->Tick();
    }
    return std::move(call_handler.value());
  }

 private:
  virtual void TestImpl() = 0;

  ::testing::StrictMock<MockAcceptor> acceptor_;
  std::unique_ptr<TransportFixture> fixture_;
  TransportFixture::ClientAndServerTransportPair transport_pair_ =
      fixture_->CreateTransportPair();
  std::shared_ptr<grpc_event_engine::experimental::FuzzingEventEngine>
      event_engine_{
          std::make_shared<grpc_event_engine::experimental::FuzzingEventEngine>(
              []() {
                grpc_timer_manager_set_threading(false);
                grpc_event_engine::experimental::FuzzingEventEngine::Options
                    options;
                return options;
              }(),
              fuzzing_event_engine::Actions())};
  MemoryAllocator allocator_ = MakeResourceQuota("test-quota")
                                   ->memory_quota()
                                   ->CreateMemoryAllocator("test-allocator");
};

class TransportTestRegistry {
 public:
  static TransportTestRegistry& Get();
  void RegisterTest(absl::string_view name,
                    absl::AnyInvocable<TransportTest*(
                        std::unique_ptr<grpc_core::TransportFixture>) const>
                        create);

  struct Test {
    absl::string_view name;
    absl::AnyInvocable<TransportTest*(
        std::unique_ptr<grpc_core::TransportFixture>) const>
        create;
  };

  const std::vector<Test>& tests() const { return tests_; }

 private:
  std::vector<Test> tests_;
};

}  // namespace grpc_core

#define TRANSPORT_TEST(name)                                                 \
  class TransportTest_##name : public grpc_core::TransportTest {             \
   public:                                                                   \
    using TransportTest::TransportTest;                                      \
    void TestBody() override { RunTest(); }                                  \
                                                                             \
   private:                                                                  \
    void TestImpl() override;                                                \
    static grpc_core::TransportTest* Create(                                 \
        std::unique_ptr<grpc_core::TransportFixture> fixture) {              \
      return new TransportTest_##name(std::move(fixture));                   \
    }                                                                        \
    static int registered_;                                                  \
  };                                                                         \
  int TransportTest_##name::registered_ =                                    \
      (grpc_core::TransportTestRegistry::Get().RegisterTest(#name, &Create), \
       0);                                                                   \
  void TransportTest_##name::TestImpl()

#endif
