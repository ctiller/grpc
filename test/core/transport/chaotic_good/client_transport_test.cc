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

#include "src/core/ext/transport/chaotic_good/client_transport.h"

#include <algorithm>
#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "absl/functional/any_invocable.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "absl/types/optional.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <grpc/event_engine/event_engine.h>
#include <grpc/event_engine/memory_allocator.h>
#include <grpc/event_engine/slice.h>
#include <grpc/event_engine/slice_buffer.h>
#include <grpc/grpc.h>
#include <grpc/status.h>

#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/iomgr/timer_manager.h"
#include "src/core/lib/promise/activity.h"
#include "src/core/lib/promise/event_engine_wakeup_scheduler.h"
#include "src/core/lib/promise/if.h"
#include "src/core/lib/promise/join.h"
#include "src/core/lib/promise/loop.h"
#include "src/core/lib/promise/map.h"
#include "src/core/lib/promise/pipe.h"
#include "src/core/lib/promise/seq.h"
#include "src/core/lib/resource_quota/arena.h"
#include "src/core/lib/resource_quota/memory_quota.h"
#include "src/core/lib/resource_quota/resource_quota.h"
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/slice/slice_internal.h"
#include "src/core/lib/transport/metadata_batch.h"
#include "test/core/event_engine/fuzzing_event_engine/fuzzing_event_engine.h"
#include "test/core/event_engine/fuzzing_event_engine/fuzzing_event_engine.pb.h"

using testing::MockFunction;
using testing::Return;
using testing::Sequence;
using testing::StrictMock;
using testing::WithArgs;

using EventEngineSlice = grpc_event_engine::experimental::Slice;
using grpc_event_engine::experimental::EventEngine;
using grpc_event_engine::experimental::internal::SliceCast;

namespace grpc_core {
namespace chaotic_good {
namespace testing {

class MockEndpoint : public EventEngine::Endpoint {
 public:
  MOCK_METHOD(bool, Read,
              (absl::AnyInvocable<void(absl::Status)> on_read,
               grpc_event_engine::experimental::SliceBuffer* buffer,
               const EventEngine::Endpoint::ReadArgs* args),
              (override));

  MOCK_METHOD(bool, Write,
              (absl::AnyInvocable<void(absl::Status)> on_writable,
               grpc_event_engine::experimental::SliceBuffer* data,
               const EventEngine::Endpoint::WriteArgs* args),
              (override));

  MOCK_METHOD(const EventEngine::ResolvedAddress&, GetPeerAddress, (),
              (const, override));
  MOCK_METHOD(const EventEngine::ResolvedAddress&, GetLocalAddress, (),
              (const, override));
};

struct MockPromiseEndpoint {
  StrictMock<MockEndpoint>* endpoint = new StrictMock<MockEndpoint>();
  std::unique_ptr<PromiseEndpoint> promise_endpoint =
      std::make_unique<PromiseEndpoint>(
          std::unique_ptr<StrictMock<MockEndpoint>>(endpoint), SliceBuffer());
  Sequence read_sequence;
  Sequence write_sequence;
  void ExpectRead(std::initializer_list<EventEngineSlice> slices_init,
                  EventEngine* schedule_on_event_engine) {
    std::vector<EventEngineSlice> slices;
    for (auto&& slice : slices_init) slices.emplace_back(slice.Copy());
    EXPECT_CALL(*endpoint, Read)
        .InSequence(read_sequence)
        .WillOnce(WithArgs<0, 1>(
            [slices = std::move(slices), schedule_on_event_engine](
                absl::AnyInvocable<void(absl::Status)> on_read,
                grpc_event_engine::experimental::SliceBuffer* buffer) mutable {
              for (auto& slice : slices) {
                buffer->Append(std::move(slice));
              }
              if (schedule_on_event_engine != nullptr) {
                schedule_on_event_engine->Run(
                    [on_read = std::move(on_read)]() mutable {
                      on_read(absl::OkStatus());
                    });
                return false;
              } else {
                return true;
              }
            }));
  }
  void ExpectWrite(std::initializer_list<EventEngineSlice> slices,
                   EventEngine* schedule_on_event_engine) {
    SliceBuffer expect;
    for (auto&& slice : slices) {
      expect.Append(SliceCast<Slice>(slice.Copy()));
    }
    EXPECT_CALL(*endpoint, Write)
        .InSequence(write_sequence)
        .WillOnce(WithArgs<0, 1>(
            [expect = expect.JoinIntoString(), schedule_on_event_engine](
                absl::AnyInvocable<void(absl::Status)> on_writable,
                grpc_event_engine::experimental::SliceBuffer* buffer) mutable {
              SliceBuffer tmp;
              grpc_slice_buffer_swap(buffer->c_slice_buffer(),
                                     tmp.c_slice_buffer());
              EXPECT_EQ(tmp.JoinIntoString(), expect);
              if (schedule_on_event_engine != nullptr) {
                schedule_on_event_engine->Run(
                    [on_writable = std::move(on_writable)]() mutable {
                      on_writable(absl::OkStatus());
                    });
                return false;
              } else {
                return true;
              }
            }));
  }
};

// Encoded string of header ":path: /demo.Service/Step".
const uint8_t kPathDemoServiceStep[] = {
    0x40, 0x05, 0x3a, 0x70, 0x61, 0x74, 0x68, 0x12, 0x2f,
    0x64, 0x65, 0x6d, 0x6f, 0x2e, 0x53, 0x65, 0x72, 0x76,
    0x69, 0x63, 0x65, 0x2f, 0x53, 0x74, 0x65, 0x70};

// Encoded string of trailer "grpc-status: 0".
const uint8_t kGrpcStatus0[] = {0x10, 0x0b, 0x67, 0x72, 0x70, 0x63, 0x2d, 0x73,
                                0x74, 0x61, 0x74, 0x75, 0x73, 0x01, 0x30};

ClientMetadataHandle TestInitialMetadata() {
  auto md =
      GetContext<Arena>()->MakePooled<ClientMetadata>(GetContext<Arena>());
  md->Set(HttpPathMetadata(), Slice::FromStaticString("/demo.Service/Step"));
  return md;
}

EventEngineSlice SerializedFrameHeader(FrameType type, uint8_t flags,
                                       uint32_t stream_id,
                                       uint32_t header_length,
                                       uint32_t message_length,
                                       uint32_t message_padding,
                                       uint32_t trailer_length) {
  uint8_t buffer[24] = {static_cast<uint8_t>(type),
                        flags,
                        0,
                        0,
                        static_cast<uint8_t>(stream_id),
                        static_cast<uint8_t>(stream_id >> 8),
                        static_cast<uint8_t>(stream_id >> 16),
                        static_cast<uint8_t>(stream_id >> 24),
                        static_cast<uint8_t>(header_length),
                        static_cast<uint8_t>(header_length >> 8),
                        static_cast<uint8_t>(header_length >> 16),
                        static_cast<uint8_t>(header_length >> 24),
                        static_cast<uint8_t>(message_length),
                        static_cast<uint8_t>(message_length >> 8),
                        static_cast<uint8_t>(message_length >> 16),
                        static_cast<uint8_t>(message_length >> 24),
                        static_cast<uint8_t>(message_padding),
                        static_cast<uint8_t>(message_padding >> 8),
                        static_cast<uint8_t>(message_padding >> 16),
                        static_cast<uint8_t>(message_padding >> 24),
                        static_cast<uint8_t>(trailer_length),
                        static_cast<uint8_t>(trailer_length >> 8),
                        static_cast<uint8_t>(trailer_length >> 16),
                        static_cast<uint8_t>(trailer_length >> 24)};
  return EventEngineSlice::FromCopiedBuffer(buffer, 24);
}

EventEngineSlice Zeros(uint32_t length) {
  std::string zeros(length, 0);
  return EventEngineSlice::FromCopiedBuffer(zeros.data(), length);
}

// Send messages from client to server.
auto SendClientToServerMessages(CallInitiator initiator, int num_messages) {
  return Loop([initiator, num_messages, i = 0]() mutable {
    bool has_message = (i < num_messages);
    return If(
        has_message,
        Seq(initiator.PushMessage(GetContext<Arena>()->MakePooled<Message>(
                SliceBuffer(Slice::FromCopiedString(std::to_string(i))), 0)),
            [&i]() -> LoopCtl<absl::Status> {
              ++i;
              return Continue();
            }),
        [initiator]() mutable -> LoopCtl<absl::Status> {
          initiator.FinishSends();
          return absl::OkStatus();
        });
  });
}

class ClientTransportTest : public ::testing::Test {
 protected:
  const std::shared_ptr<grpc_event_engine::experimental::FuzzingEventEngine>&
  event_engine() {
    return event_engine_;
  }

  MemoryAllocator* memory_allocator() { return &allocator_; }

 private:
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

TEST_F(ClientTransportTest, AddOneStream) {
  MockPromiseEndpoint control_endpoint;
  MockPromiseEndpoint data_endpoint;
  control_endpoint.ExpectRead(
      {SerializedFrameHeader(FrameType::kFragment, 7, 1, 26, 8, 56, 15),
       EventEngineSlice::FromCopiedBuffer(kPathDemoServiceStep,
                                          sizeof(kPathDemoServiceStep)),
       EventEngineSlice::FromCopiedBuffer(kGrpcStatus0, sizeof(kGrpcStatus0))},
      event_engine().get());
  data_endpoint.ExpectRead(
      {EventEngineSlice::FromCopiedString("12345678"), Zeros(56)}, nullptr);
  EXPECT_CALL(*control_endpoint.endpoint, Read)
      .InSequence(control_endpoint.read_sequence)
      .WillOnce(Return(false));
  auto transport = MakeOrphanable<ChaoticGoodClientTransport>(
      std::move(control_endpoint.promise_endpoint),
      std::move(data_endpoint.promise_endpoint), event_engine());
  auto call = MakeCall(event_engine().get(), 8192, memory_allocator());
  transport->StartCall(std::move(call.handler));
  StrictMock<MockFunction<void()>> on_done;
  EXPECT_CALL(on_done, Call());
  control_endpoint.ExpectWrite(
      {SerializedFrameHeader(FrameType::kFragment, 1, 1,
                             sizeof(kPathDemoServiceStep), 0, 0, 0),
       EventEngineSlice::FromCopiedBuffer(kPathDemoServiceStep,
                                          sizeof(kPathDemoServiceStep))},
      nullptr);
  control_endpoint.ExpectWrite(
      {SerializedFrameHeader(FrameType::kFragment, 2, 1, 0, 1, 63, 0)},
      nullptr);
  data_endpoint.ExpectWrite(
      {EventEngineSlice::FromCopiedString("0"), Zeros(63)}, nullptr);
  control_endpoint.ExpectWrite(
      {SerializedFrameHeader(FrameType::kFragment, 4, 1, 0, 0, 0, 0)}, nullptr);
  call.initiator.SpawnGuarded("test-send", [initiator =
                                                call.initiator]() mutable {
    return TrySeq(initiator.PushClientInitialMetadata(TestInitialMetadata()),
                  SendClientToServerMessages(initiator, 1));
  });
  call.initiator.SpawnInfallible(
      "test-read", [&on_done, initiator = call.initiator]() mutable {
        return Seq(
            initiator.PullServerInitialMetadata(),
            [](ValueOrFailure<ServerMetadataHandle> md) {
              EXPECT_TRUE(md.ok());
              EXPECT_EQ(
                  md.value()->get_pointer(HttpPathMetadata())->as_string_view(),
                  "/demo.Service/Step");
              return Empty{};
            },
            initiator.PullMessage(),
            [](NextResult<MessageHandle> msg) {
              EXPECT_TRUE(msg.has_value());
              EXPECT_EQ(msg.value()->payload()->JoinIntoString(), "12345678");
              return Empty{};
            },
            initiator.PullMessage(),
            [](NextResult<MessageHandle> msg) {
              EXPECT_FALSE(msg.has_value());
              return Empty{};
            },
            initiator.PullServerTrailingMetadata(),
            [&on_done](ServerMetadataHandle md) {
              EXPECT_EQ(md->get(GrpcStatusMetadata()).value(), GRPC_STATUS_OK);
              on_done.Call();
              return Empty{};
            });
      });
  // Wait until ClientTransport's internal activities to finish.
  event_engine()->TickUntilIdle();
  event_engine()->UnsetGlobalHooks();
}

TEST_F(ClientTransportTest, AddOneStreamMultipleMessages) {
  MockPromiseEndpoint control_endpoint;
  MockPromiseEndpoint data_endpoint;
  control_endpoint.ExpectRead(
      {SerializedFrameHeader(FrameType::kFragment, 3, 1, 26, 8, 56, 0),
       EventEngineSlice::FromCopiedBuffer(kPathDemoServiceStep,
                                          sizeof(kPathDemoServiceStep))},
      event_engine().get());
  control_endpoint.ExpectRead(
      {SerializedFrameHeader(FrameType::kFragment, 6, 1, 0, 8, 56, 15),
       EventEngineSlice::FromCopiedBuffer(kGrpcStatus0, sizeof(kGrpcStatus0))},
      event_engine().get());
  data_endpoint.ExpectRead(
      {EventEngineSlice::FromCopiedString("12345678"), Zeros(56)}, nullptr);
  data_endpoint.ExpectRead(
      {EventEngineSlice::FromCopiedString("87654321"), Zeros(56)}, nullptr);
  EXPECT_CALL(*control_endpoint.endpoint, Read)
      .InSequence(control_endpoint.read_sequence)
      .WillOnce(Return(false));
  auto transport = MakeOrphanable<ChaoticGoodClientTransport>(
      std::move(control_endpoint.promise_endpoint),
      std::move(data_endpoint.promise_endpoint), event_engine());
  auto call = MakeCall(event_engine().get(), 8192, memory_allocator());
  transport->StartCall(std::move(call.handler));
  StrictMock<MockFunction<void()>> on_done;
  EXPECT_CALL(on_done, Call());
  control_endpoint.ExpectWrite(
      {SerializedFrameHeader(FrameType::kFragment, 1, 1,
                             sizeof(kPathDemoServiceStep), 0, 0, 0),
       EventEngineSlice::FromCopiedBuffer(kPathDemoServiceStep,
                                          sizeof(kPathDemoServiceStep))},
      nullptr);
  control_endpoint.ExpectWrite(
      {SerializedFrameHeader(FrameType::kFragment, 2, 1, 0, 1, 63, 0)},
      nullptr);
  data_endpoint.ExpectWrite(
      {EventEngineSlice::FromCopiedString("0"), Zeros(63)}, nullptr);
  control_endpoint.ExpectWrite(
      {SerializedFrameHeader(FrameType::kFragment, 2, 1, 0, 1, 63, 0)},
      nullptr);
  data_endpoint.ExpectWrite(
      {EventEngineSlice::FromCopiedString("1"), Zeros(63)}, nullptr);
  control_endpoint.ExpectWrite(
      {SerializedFrameHeader(FrameType::kFragment, 4, 1, 0, 0, 0, 0)}, nullptr);
  call.initiator.SpawnGuarded("test-send", [initiator =
                                                call.initiator]() mutable {
    return TrySeq(initiator.PushClientInitialMetadata(TestInitialMetadata()),
                  SendClientToServerMessages(initiator, 2));
  });
  call.initiator.SpawnInfallible(
      "test-read", [&on_done, initiator = call.initiator]() mutable {
        return Seq(
            initiator.PullServerInitialMetadata(),
            [](ValueOrFailure<ServerMetadataHandle> md) {
              EXPECT_TRUE(md.ok());
              EXPECT_EQ(
                  md.value()->get_pointer(HttpPathMetadata())->as_string_view(),
                  "/demo.Service/Step");
              return Empty{};
            },
            initiator.PullMessage(),
            [](NextResult<MessageHandle> msg) {
              EXPECT_TRUE(msg.has_value());
              EXPECT_EQ(msg.value()->payload()->JoinIntoString(), "12345678");
              return Empty{};
            },
            initiator.PullMessage(),
            [](NextResult<MessageHandle> msg) {
              EXPECT_TRUE(msg.has_value());
              EXPECT_EQ(msg.value()->payload()->JoinIntoString(), "87654321");
              return Empty{};
            },
            initiator.PullMessage(),
            [](NextResult<MessageHandle> msg) {
              EXPECT_FALSE(msg.has_value());
              return Empty{};
            },
            initiator.PullServerTrailingMetadata(),
            [&on_done](ServerMetadataHandle md) {
              EXPECT_EQ(md->get(GrpcStatusMetadata()).value(), GRPC_STATUS_OK);
              on_done.Call();
              return Empty{};
            });
      });
  // Wait until ClientTransport's internal activities to finish.
  event_engine()->TickUntilIdle();
  event_engine()->UnsetGlobalHooks();
}

}  // namespace testing
}  // namespace chaotic_good
}  // namespace grpc_core

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  // Must call to create default EventEngine.
  grpc_init();
  int ret = RUN_ALL_TESTS();
  grpc_shutdown();
  return ret;
}
