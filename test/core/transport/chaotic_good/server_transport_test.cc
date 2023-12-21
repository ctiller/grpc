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

#include "src/core/ext/transport/chaotic_good/server_transport.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "absl/functional/any_invocable.h"
#include "absl/status/status.h"
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
#include "src/core/lib/promise/seq.h"
#include "src/core/lib/resource_quota/arena.h"
#include "src/core/lib/resource_quota/memory_quota.h"
#include "src/core/lib/resource_quota/resource_quota.h"
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/slice/slice_internal.h"
#include "src/core/lib/transport/metadata_batch.h"
#include "test/core/event_engine/fuzzing_event_engine/fuzzing_event_engine.h"
#include "test/core/event_engine/fuzzing_event_engine/fuzzing_event_engine.pb.h"
#include "test/core/transport/chaotic_good/mock_promise_endpoint.h"
#include "test/core/transport/chaotic_good/transport_test.h"

using testing::_;
using testing::MockFunction;
using testing::Return;
using testing::Sequence;
using testing::StrictMock;
using testing::WithArgs;

using EventEngineSlice = grpc_event_engine::experimental::Slice;
using grpc_event_engine::experimental::EventEngine;

namespace grpc_core {
namespace chaotic_good {
namespace testing {

// Encoded string of header ":path: /demo.Service/Step".
const uint8_t kPathDemoServiceStep[] = {
    0x40, 0x05, 0x3a, 0x70, 0x61, 0x74, 0x68, 0x12, 0x2f,
    0x64, 0x65, 0x6d, 0x6f, 0x2e, 0x53, 0x65, 0x72, 0x76,
    0x69, 0x63, 0x65, 0x2f, 0x53, 0x74, 0x65, 0x70};

class MockAcceptor : public ServerTransport::Acceptor {
 public:
  MOCK_METHOD(Arena*, CreateArena, (), (override));
  MOCK_METHOD(absl::StatusOr<CallInitiator>, CreateCall,
              (ClientMetadata & client_initial_metadata, Arena* arena),
              (override));
};

TEST_F(TransportTest, ReadAndWriteOneMessage) {
  MockPromiseEndpoint control_endpoint;
  MockPromiseEndpoint data_endpoint;
  StrictMock<MockAcceptor> acceptor;
  auto transport = MakeOrphanable<ChaoticGoodServerTransport>(
      CoreConfiguration::Get()
          .channel_args_preconditioning()
          .PreconditionChannelArgs(nullptr),
      std::move(control_endpoint.promise_endpoint),
      std::move(data_endpoint.promise_endpoint), event_engine());
  control_endpoint.ExpectRead(
      {SerializedFrameHeader(FrameType::kFragment, 7, 1, 26, 8, 56, 0),
       EventEngineSlice::FromCopiedBuffer(kPathDemoServiceStep,
                                          sizeof(kPathDemoServiceStep))},
      event_engine().get());
  data_endpoint.ExpectRead(
      {EventEngineSlice::FromCopiedString("12345678"), Zeros(56)}, nullptr);
  EXPECT_CALL(*control_endpoint.endpoint, Read)
      .InSequence(control_endpoint.read_sequence)
      .WillOnce(Return(false));
  auto* call_arena = Arena::Create(1024, memory_allocator());
  CallInitiatorAndHandler call = MakeCall(event_engine().get(), call_arena);
  EXPECT_CALL(acceptor, CreateArena).WillOnce(Return(call_arena));
  EXPECT_CALL(acceptor, CreateCall(_, call_arena))
      .WillOnce(WithArgs<0>([call_initiator = std::move(call.initiator)](
                                ClientMetadata& client_initial_metadata) {
        EXPECT_EQ(client_initial_metadata.get_pointer(HttpPathMetadata())
                      ->as_string_view(),
                  "/demo.Service/Step");
        return call_initiator;
      }));
  transport->SetAcceptor(&acceptor);
  StrictMock<MockFunction<void()>> on_done;
  EXPECT_CALL(on_done, Call());
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
