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
  Sequence sequence;
  void ExpectRead(std::initializer_list<EventEngineSlice> slices_init,
                  EventEngine* schedule_on_event_engine) {
    std::vector<EventEngineSlice> slices;
    for (auto&& slice : slices_init) slices.emplace_back(slice.Copy());
    EXPECT_CALL(*endpoint, Read)
        .InSequence(sequence)
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
        .InSequence(sequence)
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
    0x10, 0x05, 0x3a, 0x70, 0x61, 0x74, 0x68, 0x12, 0x2f,
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
  return Loop([initiator, num_messages]() mutable {
    bool has_message = (num_messages > 0);
    return If(
        has_message,
        Seq(initiator.PushMessage(GetContext<Arena>()->MakePooled<Message>()),
            [&num_messages]() -> LoopCtl<absl::Status> {
              --num_messages;
              return Continue();
            }),
        [initiator]() mutable -> LoopCtl<absl::Status> {
          initiator.FinishSends();
          return absl::OkStatus();
        });
  });
}

class ClientTransportTest : public ::testing::Test {
 public:
#if 0
  // Add stream into client transport, and expect return trailers of
  // "grpc-status:code".
  auto AddStream(CallArgs args, const grpc_status_code trailers) {
    return Seq(client_transport_->AddStream(std::move(args)),
               [trailers](ServerMetadataHandle ret) {
                 // AddStream will finish with server trailers:
                 // "grpc-status:code".
                 EXPECT_EQ(ret->get(GrpcStatusMetadata()).value(), trailers);
                 return trailers;
               });
  }
  // Start read from control endpoints.
  auto StartRead(const absl::Status& read_status) {
    return [read_status, this] {
      read_callback_(read_status);
      return read_status;
    };
  }
  // Receive messages from server to client.
  auto ReceiveServerToClientMessages(
      Pipe<ServerMetadataHandle>& pipe_server_intial_metadata,
      Pipe<MessageHandle>& pipe_server_to_client_messages) {
    return Seq(
        // Receive server initial metadata.
        Map(pipe_server_intial_metadata.receiver.Next(),
            [](NextResult<ServerMetadataHandle> r) {
              // Expect value: ":path: /demo.Service/Step"
              EXPECT_TRUE(r.has_value());
              EXPECT_EQ(
                  r.value()->get_pointer(HttpPathMetadata())->as_string_view(),
                  "/demo.Service/Step");
              return absl::OkStatus();
            }),
        // Receive server to client messages.
        Map(pipe_server_to_client_messages.receiver.Next(),
            [this](NextResult<MessageHandle> r) {
              EXPECT_TRUE(r.has_value());
              EXPECT_EQ(r.value()->payload()->JoinIntoString(), message_);
              return absl::OkStatus();
            }),
        [&pipe_server_intial_metadata,
         &pipe_server_to_client_messages]() mutable {
          // Close pipes after receive message.
          pipe_server_to_client_messages.sender.Close();
          pipe_server_intial_metadata.sender.Close();
          return absl::OkStatus();
        });
  }
#endif

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
      {Zeros(56), EventEngineSlice::FromCopiedString("12345678")}, nullptr);
  EXPECT_CALL(*control_endpoint.endpoint, Read)
      .InSequence(control_endpoint.sequence)
      .WillOnce(Return(false));
  auto transport = MakeOrphanable<ClientTransport>(
      std::move(control_endpoint.promise_endpoint),
      std::move(data_endpoint.promise_endpoint), event_engine());
  auto call = MakeCall(event_engine().get(), 8192, memory_allocator());
  transport->StartCall(std::move(call.handler));
  StrictMock<MockFunction<void()>> on_done;
  EXPECT_CALL(on_done, Call());
  control_endpoint.ExpectWrite(
      {SerializedFrameHeader(FrameType::kFragment, 1,
                             sizeof(kPathDemoServiceStep), 0, 0, 0, 0),
       EventEngineSlice::FromCopiedBuffer(kPathDemoServiceStep,
                                          sizeof(kPathDemoServiceStep))},
      nullptr);
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

#if 0
TEST_F(ClientTransportTest, AddOneStreamMultipleMessages) {
  InitialClientTransport(1);
  ClientMetadataHandle md;
  auto args = CallArgs{std::move(md),
                       ClientInitialMetadataOutstandingToken::Empty(),
                       nullptr,
                       &pipe_server_intial_metadata_.sender,
                       &pipe_client_to_server_messages_.receiver,
                       &pipe_server_to_client_messages_.sender};
  StrictMock<MockFunction<void(absl::Status)>> on_done;
  EXPECT_CALL(on_done, Call(absl::OkStatus()));
  EXPECT_CALL(control_endpoint_, Write).Times(3).WillRepeatedly(Return(true));
  EXPECT_CALL(data_endpoint_, Write).Times(3).WillRepeatedly(Return(true));
  auto activity = MakeActivity(
      Seq(
          // Concurrently: write and read messages in client transport.
          Join(
              // Add first stream with call_args into client transport.
              AddStream(std::move(args), GRPC_STATUS_OK),
              // Start read from control endpoints.
              StartRead(absl::OkStatus()),
              // Send messages to call_args.client_to_server_messages pipe,
              // which will be eventually sent to control/data endpoints.
              SendClientToServerMessages(pipe_client_to_server_messages_, 3),
              // Receive messages from control/data endpoints.
              ReceiveServerToClientMessages(pipe_server_intial_metadata_,
                                            pipe_server_to_client_messages_)),
          // Once complete, verify successful sending and the received value.
          [](const std::tuple<grpc_status_code, absl::Status, absl::Status,
                              absl::Status>& ret) {
            EXPECT_EQ(std::get<0>(ret), GRPC_STATUS_OK);
            EXPECT_TRUE(std::get<1>(ret).ok());
            EXPECT_TRUE(std::get<2>(ret).ok());
            EXPECT_TRUE(std::get<3>(ret).ok());
            return absl::OkStatus();
          }),
      EventEngineWakeupScheduler(event_engine_),
      [&on_done](absl::Status status) { on_done.Call(std::move(status)); });
  // Wait until ClientTransport's internal activities to finish.
  event_engine_->TickUntilIdle();
  event_engine_->UnsetGlobalHooks();
}

TEST_F(ClientTransportTest, AddMultipleStreamsMultipleMessages) {
  InitialClientTransport(2);
  ClientMetadataHandle first_stream_md;
  ClientMetadataHandle second_stream_md;
  auto first_stream_args =
      CallArgs{std::move(first_stream_md),
               ClientInitialMetadataOutstandingToken::Empty(),
               nullptr,
               &pipe_server_intial_metadata_.sender,
               &pipe_client_to_server_messages_.receiver,
               &pipe_server_to_client_messages_.sender};
  auto second_stream_args =
      CallArgs{std::move(second_stream_md),
               ClientInitialMetadataOutstandingToken::Empty(),
               nullptr,
               &pipe_server_intial_metadata_second_.sender,
               &pipe_client_to_server_messages_second_.receiver,
               &pipe_server_to_client_messages_second_.sender};
  StrictMock<MockFunction<void(absl::Status)>> on_done;
  EXPECT_CALL(on_done, Call(absl::OkStatus()));
  EXPECT_CALL(control_endpoint_, Write).Times(6).WillRepeatedly(Return(true));
  EXPECT_CALL(data_endpoint_, Write).Times(6).WillRepeatedly(Return(true));
  auto activity = MakeActivity(
      Seq(
          // Concurrently: write and read messages from client transport.
          Join(
              // Add first stream with call_args into client transport.
              AddStream(std::move(first_stream_args), GRPC_STATUS_OK),
              // Start read from control endpoints.
              StartRead(absl::OkStatus()),
              // Send messages to first stream's
              // call_args.client_to_server_messages pipe, which will be
              // eventually sent to control/data endpoints.
              SendClientToServerMessages(pipe_client_to_server_messages_, 3),
              // Receive first stream's messages from control/data endpoints.
              ReceiveServerToClientMessages(pipe_server_intial_metadata_,
                                            pipe_server_to_client_messages_)),
          Join(
              // Add second stream with call_args into client transport.
              AddStream(std::move(second_stream_args), GRPC_STATUS_OK),
              // Send messages to second stream's
              // call_args.client_to_server_messages pipe, which will be
              // eventually sent to control/data endpoints.
              SendClientToServerMessages(pipe_client_to_server_messages_second_,
                                         3),
              // Receive second stream's messages from control/data endpoints.
              ReceiveServerToClientMessages(
                  pipe_server_intial_metadata_second_,
                  pipe_server_to_client_messages_second_)),
          // Once complete, verify successful sending and the received value.
          [](const std::tuple<grpc_status_code, absl::Status, absl::Status>&
                 ret) {
            EXPECT_EQ(std::get<0>(ret), GRPC_STATUS_OK);
            EXPECT_TRUE(std::get<1>(ret).ok());
            EXPECT_TRUE(std::get<2>(ret).ok());
            return absl::OkStatus();
          }),
      EventEngineWakeupScheduler(event_engine_),
      [&on_done](absl::Status status) { on_done.Call(std::move(status)); });
  // Wait until ClientTransport's internal activities to finish.
  event_engine_->TickUntilIdle();
  event_engine_->UnsetGlobalHooks();
}
#endif

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
