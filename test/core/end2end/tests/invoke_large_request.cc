//
//
// Copyright 2015 gRPC authors.
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

#include <string.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <grpc/byte_buffer.h>
#include <grpc/grpc.h>
#include <grpc/impl/propagation_bits.h>
#include <grpc/slice.h>
#include <grpc/status.h>
#include <grpc/support/log.h>
#include <grpc/support/time.h>

#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/debug/event_log.h"
#include "src/core/lib/gpr/useful.h"
#include "src/core/lib/gprpp/no_destruct.h"
#include "src/core/lib/gprpp/time.h"
#include "src/core/lib/iomgr/exec_ctx.h"
#include "test/core/end2end/cq_verifier.h"
#include "test/core/end2end/end2end_tests.h"
#include "test/core/util/test_config.h"

namespace grpc_core {
namespace {

TEST_P(CoreLargeSendTest, InvokeLargeRequest) {
  const size_t kMessageSize = 10 * 1024 * 1024;
  InitServer(
      ChannelArgs().Set(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, kMessageSize));
  InitClient(
      ChannelArgs().Set(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, kMessageSize));
  auto c = NewClientCall("/foo").Timeout(Duration::Seconds(300)).Create();
  auto send_from_client = RandomSlice(kMessageSize);
  auto send_from_server = RandomSlice(kMessageSize);
  CoreEnd2endTest::IncomingStatusOnClient server_status;
  CoreEnd2endTest::IncomingMetadata server_initial_metadata;
  CoreEnd2endTest::IncomingMessage server_message;
  c.NewBatch(1)
      .SendInitialMetadata({})
      .SendMessage(send_from_client.Ref())
      .SendCloseFromClient()
      .RecvInitialMetadata(server_initial_metadata)
      .RecvMessage(server_message)
      .RecvStatusOnClient(server_status);
  auto s = RequestCall(101);
  Expect(101, true);
  Step(Duration::Minutes(1));
  CoreEnd2endTest::IncomingMessage client_message;
  s.NewBatch(102).SendInitialMetadata({}).RecvMessage(client_message);
  Expect(102, true);
  Step(Duration::Minutes(1));
  CoreEnd2endTest::IncomingCloseOnServer client_close;
  s.NewBatch(103)
      .SendStatusFromServer(GRPC_STATUS_UNIMPLEMENTED, "xyz", {})
      .SendMessage(send_from_server.Ref())
      .RecvCloseOnServer(client_close);
  Expect(103, true);
  Expect(1, true);
  Step(Duration::Minutes(1));
  EXPECT_EQ(server_status.status(), GRPC_STATUS_UNIMPLEMENTED);
  EXPECT_EQ(server_status.message(), "xyz");
  EXPECT_EQ(s.method(), "/foo");
  EXPECT_FALSE(client_close.was_cancelled());
  EXPECT_EQ(client_message.payload(), send_from_client);
  EXPECT_EQ(server_message.payload(), send_from_server);
}

}  // namespace
}  // namespace grpc_core
