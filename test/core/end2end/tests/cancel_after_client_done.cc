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

#include "cancel_test_helpers.h"
#include "gmock/gmock.h"

#include <grpc/byte_buffer.h>
#include <grpc/grpc.h>
#include <grpc/impl/propagation_bits.h>
#include <grpc/slice.h>
#include <grpc/status.h>
#include <grpc/support/log.h>
#include <grpc/support/time.h>

#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/gpr/useful.h"
#include "test/core/end2end/cq_verifier.h"
#include "test/core/end2end/end2end_tests.h"
#include "test/core/end2end/tests/cancel_test_helpers.h"
#include "test/core/util/test_config.h"

namespace grpc_core {

// Cancel after accept with a writes closed, no payload
void CancelAfterClientDone(
    CoreEnd2endTest& test,
    std::unique_ptr<CancellationMode> cancellation_mode) {
  auto c = test.NewClientCall("/service/method")
               .Timeout(Duration::Seconds(5))
               .Create();
  CoreEnd2endTest::IncomingStatusOnClient server_status;
  CoreEnd2endTest::IncomingMetadata server_initial_metadata;
  CoreEnd2endTest::IncomingMessage server_message;
  c.NewBatch(1)
      .RecvStatusOnClient(server_status)
      .SendInitialMetadata({})
      .SendMessage(RandomSlice(1024))
      .RecvInitialMetadata(server_initial_metadata)
      .RecvMessage(server_message)
      .SendCloseFromClient();
  auto s = test.RequestCall(2);
  test.Expect(2, true);
  test.Step();
  CoreEnd2endTest::IncomingMessage client_message;
  CoreEnd2endTest::IncomingCloseOnServer client_close;
  s.NewBatch(3)
      .RecvMessage(client_message)
      .SendInitialMetadata({})
      .SendMessage(RandomSlice(1024))
      .RecvCloseOnServer(client_close);
  cancellation_mode->Apply(c.c_call());
  test.Expect(1, true);
  test.Expect(3, true);
  test.Step();
  EXPECT_THAT(server_status.status(),
              ::testing::AnyOf(cancellation_mode->ExpectedStatus(),
                               GRPC_STATUS_INTERNAL));
  EXPECT_TRUE(client_close.was_cancelled());
}

TEST_P(CoreEnd2endTest, CancelAfterClientDone) {
  CancelAfterClientDone(*this, std::make_unique<CancelCancellationMode>());
}

TEST_P(CoreDeadlineTest, DeadlineAfterClientDone) {
  CancelAfterClientDone(*this, std::make_unique<DeadlineCancellationMode>());
}

}  // namespace grpc_core