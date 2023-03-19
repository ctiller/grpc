//
//
// Copyright 2016 gRPC authors.
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

#include <limits.h>

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "absl/status/status.h"
#include "absl/types/optional.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <grpc/status.h>

#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/channel/channel_fwd.h"
#include "src/core/lib/channel/channel_stack.h"
#include "src/core/lib/channel/channel_stack_builder.h"
#include "src/core/lib/config/core_configuration.h"
#include "src/core/lib/gprpp/status_helper.h"
#include "src/core/lib/gprpp/time.h"
#include "src/core/lib/iomgr/closure.h"
#include "src/core/lib/iomgr/error.h"
#include "src/core/lib/surface/channel_stack_type.h"
#include "test/core/end2end/end2end_tests.h"

using ::testing::AnyOf;

namespace grpc_core {
namespace {

//
// Test filter - always fails to initialize a call
//

static grpc_error_handle init_call_elem(
    grpc_call_element* /*elem*/, const grpc_call_element_args* /*args*/) {
  return grpc_error_set_int(GRPC_ERROR_CREATE("access denied"),
                            grpc_core::StatusIntProperty::kRpcStatus,
                            GRPC_STATUS_PERMISSION_DENIED);
}

static void destroy_call_elem(grpc_call_element* /*elem*/,
                              const grpc_call_final_info* /*final_info*/,
                              grpc_closure* /*ignored*/) {}

static grpc_error_handle init_channel_elem(grpc_channel_element* /*elem*/,
                                           grpc_channel_element_args* args) {
  if (args->channel_args.GetBool("channel_init_fails").value_or(false)) {
    return grpc_error_set_int(
        GRPC_ERROR_CREATE("Test channel filter init error"),
        grpc_core::StatusIntProperty::kRpcStatus, GRPC_STATUS_INVALID_ARGUMENT);
  }
  return absl::OkStatus();
}

static void destroy_channel_elem(grpc_channel_element* /*elem*/) {}

static const grpc_channel_filter test_filter = {
    grpc_call_next_op,    nullptr,
    grpc_channel_next_op, 0,
    init_call_elem,       grpc_call_stack_ignore_set_pollset_or_pollset_set,
    destroy_call_elem,    0,
    init_channel_elem,    grpc_channel_stack_no_post_init,
    destroy_channel_elem, grpc_channel_next_get_info,
    "filter_init_fails"};

void RegisterFilter(grpc_channel_stack_type type) {
  grpc_core::CoreConfiguration::RegisterBuilder(
      [type](grpc_core::CoreConfiguration::Builder* builder) {
        builder->channel_init()->RegisterStage(
            type, INT_MAX, [](grpc_core::ChannelStackBuilder* builder) {
              // Want to add the filter as close to the end as possible,
              // to make sure that all of the filters work well together.
              // However, we can't add it at the very end, because either the
              // client_channel filter or connected_channel filter must be the
              // last one.  So we add it right before the last one.
              auto it = builder->mutable_stack()->end();
              --it;
              builder->mutable_stack()->insert(it, &test_filter);
              return true;
            });
      });
}

TEST_P(CoreEnd2endTest, DISABLED_ServerFilterChannelInitFails) {
  RegisterFilter(GRPC_SERVER_CHANNEL);
  InitClient(ChannelArgs());
  InitServer(ChannelArgs().Set("channel_init_fails", true));
  auto c = NewClientCall("/foo").Timeout(Duration::Seconds(5)).Create();
  CoreEnd2endTest::IncomingStatusOnClient server_status;
  CoreEnd2endTest::IncomingMetadata server_initial_metadata;
  c.NewBatch(1)
      .SendInitialMetadata({})
      .SendMessage("hello")
      .SendCloseFromClient()
      .RecvInitialMetadata(server_initial_metadata)
      .RecvStatusOnClient(server_status);
  auto s = RequestCall(101);
  Expect(1, true);
  Step();
  // Inproc channel returns invalid_argument and other clients return
  // unavailable.
  // Windows with sockpair returns unknown.
  EXPECT_THAT(server_status.status(),
              AnyOf(GRPC_STATUS_UNKNOWN, GRPC_STATUS_UNAVAILABLE,
                    GRPC_STATUS_INVALID_ARGUMENT));
  ShutdownAndDestroyServer();
};

TEST_P(CoreEnd2endTest, ServerFilterCallInitFails) {
  RegisterFilter(GRPC_SERVER_CHANNEL);
  auto c = NewClientCall("/foo").Timeout(Duration::Seconds(5)).Create();
  CoreEnd2endTest::IncomingStatusOnClient server_status;
  CoreEnd2endTest::IncomingMetadata server_initial_metadata;
  c.NewBatch(1)
      .SendInitialMetadata({})
      .SendMessage("hello")
      .SendCloseFromClient()
      .RecvInitialMetadata(server_initial_metadata)
      .RecvStatusOnClient(server_status);
  auto s = RequestCall(101);
  Expect(1, true);
  Step();
  EXPECT_EQ(server_status.status(), GRPC_STATUS_PERMISSION_DENIED);
  EXPECT_EQ(server_status.message(), "access denied");
  ShutdownAndDestroyServer();
};

TEST_P(CoreEnd2endTest, DISABLED_ClientFilterChannelInitFails) {
  RegisterFilter(GRPC_CLIENT_CHANNEL);
  RegisterFilter(GRPC_CLIENT_DIRECT_CHANNEL);
  InitServer(ChannelArgs());
  InitClient(ChannelArgs().Set("channel_init_fails", true));
  auto c = NewClientCall("/foo").Timeout(Duration::Seconds(5)).Create();
  CoreEnd2endTest::IncomingStatusOnClient server_status;
  CoreEnd2endTest::IncomingMetadata server_initial_metadata;
  c.NewBatch(1)
      .SendInitialMetadata({})
      .SendMessage("hello")
      .SendCloseFromClient()
      .RecvInitialMetadata(server_initial_metadata)
      .RecvStatusOnClient(server_status);
  Expect(1, true);
  Step();
  EXPECT_EQ(server_status.status(), GRPC_STATUS_INVALID_ARGUMENT);
}

TEST_P(CoreEnd2endTest, ClientFilterCallInitFails) {
  RegisterFilter(GRPC_CLIENT_CHANNEL);
  RegisterFilter(GRPC_CLIENT_DIRECT_CHANNEL);
  auto c = NewClientCall("/foo").Timeout(Duration::Seconds(5)).Create();
  CoreEnd2endTest::IncomingStatusOnClient server_status;
  CoreEnd2endTest::IncomingMetadata server_initial_metadata;
  CoreEnd2endTest::IncomingMessage server_message;
  c.NewBatch(1)
      .SendInitialMetadata({})
      .SendMessage("hello")
      .SendCloseFromClient()
      .RecvInitialMetadata(server_initial_metadata)
      .RecvStatusOnClient(server_status);
  Expect(1, true);
  Step();
  EXPECT_EQ(server_status.status(), GRPC_STATUS_PERMISSION_DENIED);
  EXPECT_EQ(server_status.message(), "access denied");
}

TEST_P(CoreClientChannelTest, DISABLED_SubchannelFilterChannelInitFails) {
  RegisterFilter(GRPC_CLIENT_SUBCHANNEL);
  InitServer(ChannelArgs());
  InitClient(ChannelArgs().Set("channel_init_fails", true));
  auto c = NewClientCall("/foo").Timeout(Duration::Seconds(5)).Create();
  CoreEnd2endTest::IncomingStatusOnClient server_status;
  CoreEnd2endTest::IncomingMetadata server_initial_metadata;
  CoreEnd2endTest::IncomingMessage server_message;
  c.NewBatch(1)
      .SendInitialMetadata({})
      .SendMessage("hello")
      .SendCloseFromClient()
      .RecvInitialMetadata(server_initial_metadata)
      .RecvStatusOnClient(server_status);
  Expect(1, true);
  Step();
  EXPECT_EQ(server_status.status(), GRPC_STATUS_UNAVAILABLE);
  // Create a new call.  (The first call uses a different code path in
  // client_channel.c than subsequent calls on the same channel, and we need to
  // test both.)
  auto c2 = NewClientCall("/foo").Timeout(Duration::Seconds(5)).Create();
  CoreEnd2endTest::IncomingStatusOnClient server_status2;
  CoreEnd2endTest::IncomingMetadata server_initial_metadata2;
  CoreEnd2endTest::IncomingMessage server_message2;
  c2.NewBatch(2)
      .SendInitialMetadata({})
      .SendMessage("hi again")
      .SendCloseFromClient()
      .RecvInitialMetadata(server_initial_metadata2)
      .RecvStatusOnClient(server_status2);
  Expect(2, true);
  Step();
  EXPECT_EQ(server_status2.status(), GRPC_STATUS_UNAVAILABLE);
}

TEST_P(CoreClientChannelTest, SubchannelFilterCallInitFails) {
  RegisterFilter(GRPC_CLIENT_SUBCHANNEL);
  auto c = NewClientCall("/foo").Timeout(Duration::Seconds(5)).Create();
  CoreEnd2endTest::IncomingStatusOnClient server_status;
  CoreEnd2endTest::IncomingMetadata server_initial_metadata;
  CoreEnd2endTest::IncomingMessage server_message;
  c.NewBatch(1)
      .SendInitialMetadata({})
      .SendMessage("hello")
      .SendCloseFromClient()
      .RecvInitialMetadata(server_initial_metadata)
      .RecvStatusOnClient(server_status);
  Expect(1, true);
  Step();
  EXPECT_EQ(server_status.status(), GRPC_STATUS_PERMISSION_DENIED);
  EXPECT_EQ(server_status.message(), "access denied");
  // Create a new call.  (The first call uses a different code path in
  // client_channel.c than subsequent calls on the same channel, and we need to
  // test both.)
  auto c2 = NewClientCall("/foo").Timeout(Duration::Seconds(5)).Create();
  CoreEnd2endTest::IncomingStatusOnClient server_status2;
  CoreEnd2endTest::IncomingMetadata server_initial_metadata2;
  CoreEnd2endTest::IncomingMessage server_message2;
  c2.NewBatch(2)
      .SendInitialMetadata({})
      .SendMessage("hi again")
      .SendCloseFromClient()
      .RecvInitialMetadata(server_initial_metadata2)
      .RecvStatusOnClient(server_status2);
  Expect(2, true);
  Step();
  EXPECT_EQ(server_status2.status(), GRPC_STATUS_PERMISSION_DENIED);
  EXPECT_EQ(server_status2.message(), "access denied");
}

}  // namespace
}  // namespace grpc_core
