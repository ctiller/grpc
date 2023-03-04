//
// Copyright 2017 gRPC authors.
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

#include <string.h>

#include <functional>
#include <memory>

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
#include "test/core/util/test_config.h"

static std::unique_ptr<CoreTestFixture> begin_test(
    const CoreTestConfiguration& config, const char* test_name,
    grpc_channel_args* client_args, grpc_channel_args* server_args) {
  gpr_log(GPR_INFO, "Running test: %s/%s", test_name, config.name);
  auto f = config.create_fixture(grpc_core::ChannelArgs::FromC(client_args),
                                 grpc_core::ChannelArgs::FromC(server_args));
  f->InitServer(grpc_core::ChannelArgs::FromC(server_args));
  f->InitClient(grpc_core::ChannelArgs::FromC(client_args));
  return f;
}

// Tests that we can unref a call after the first attempt starts but
// before any ops complete.  This should not cause a memory leak.
static void test_retry_cancel_after_first_attempt_starts(
    CoreTestConfiguration config) {
  grpc_call* c;
  grpc_op ops[6];
  grpc_op* op;
  grpc_metadata_array initial_metadata_recv;
  grpc_metadata_array trailing_metadata_recv;
  // Make sure to dynamically allocate the request payload slice, so
  // that the test will fail under ASAN if we don't clean up properly.
  const char request_string[] = "foo";
  grpc_slice request_payload_slice =
      grpc_slice_malloc_large(sizeof(request_string));
  memcpy(GRPC_SLICE_START_PTR(request_payload_slice), request_string,
         sizeof(request_string));
  grpc_byte_buffer* request_payload =
      grpc_raw_byte_buffer_create(&request_payload_slice, 1);
  grpc_slice_unref(request_payload_slice);
  grpc_byte_buffer* response_payload_recv = nullptr;
  grpc_status_code status;
  grpc_slice details;
  grpc_call_error error;

  grpc_arg args[] = {
      grpc_channel_arg_string_create(
          const_cast<char*>(GRPC_ARG_SERVICE_CONFIG),
          const_cast<char*>(
              "{\n"
              "  \"methodConfig\": [ {\n"
              "    \"name\": [\n"
              "      { \"service\": \"service\", \"method\": \"method\" }\n"
              "    ],\n"
              "    \"retryPolicy\": {\n"
              "      \"maxAttempts\": 3,\n"
              "      \"initialBackoff\": \"1s\",\n"
              "      \"maxBackoff\": \"120s\",\n"
              "      \"backoffMultiplier\": 1.6,\n"
              "      \"retryableStatusCodes\": [ \"ABORTED\" ]\n"
              "    }\n"
              "  } ]\n"
              "}")),
  };
  grpc_channel_args client_args = {GPR_ARRAY_SIZE(args), args};
  auto f = begin_test(config, "retry_cancel_after_first_attempt_starts",
                      &client_args, nullptr);

  grpc_core::CqVerifier cqv(f->cq());

  gpr_timespec deadline = grpc_timeout_seconds_to_deadline(5);
  c = grpc_channel_create_call(f->client(), nullptr, GRPC_PROPAGATE_DEFAULTS,
                               f->cq(),
                               grpc_slice_from_static_string("/service/method"),
                               nullptr, deadline, nullptr);
  GPR_ASSERT(c);

  grpc_metadata_array_init(&initial_metadata_recv);
  grpc_metadata_array_init(&trailing_metadata_recv);

  // Client starts send ops.
  memset(ops, 0, sizeof(ops));
  op = ops;
  op->op = GRPC_OP_SEND_INITIAL_METADATA;
  op->data.send_initial_metadata.count = 0;
  op++;
  op->op = GRPC_OP_SEND_MESSAGE;
  op->data.send_message.send_message = request_payload;
  op++;
  op->op = GRPC_OP_SEND_CLOSE_FROM_CLIENT;
  op++;
  error = grpc_call_start_batch(c, ops, static_cast<size_t>(op - ops),
                                grpc_core::CqVerifier::tag(1), nullptr);
  GPR_ASSERT(GRPC_CALL_OK == error);

  // Client starts recv_initial_metadata and recv_message, but not
  // recv_trailing_metadata.
  memset(ops, 0, sizeof(ops));
  op = ops;
  op->op = GRPC_OP_RECV_INITIAL_METADATA;
  op->data.recv_initial_metadata.recv_initial_metadata = &initial_metadata_recv;
  op++;
  op->op = GRPC_OP_RECV_MESSAGE;
  op->data.recv_message.recv_message = &response_payload_recv;
  op++;
  error = grpc_call_start_batch(c, ops, static_cast<size_t>(op - ops),
                                grpc_core::CqVerifier::tag(2), nullptr);
  GPR_ASSERT(GRPC_CALL_OK == error);

  // Client starts recv_trailing_metadata.
  memset(ops, 0, sizeof(ops));
  op = ops;
  op->op = GRPC_OP_RECV_STATUS_ON_CLIENT;
  op->data.recv_status_on_client.trailing_metadata = &trailing_metadata_recv;
  op->data.recv_status_on_client.status = &status;
  op->data.recv_status_on_client.status_details = &details;
  op++;
  error = grpc_call_start_batch(c, ops, static_cast<size_t>(op - ops),
                                grpc_core::CqVerifier::tag(3), nullptr);
  GPR_ASSERT(GRPC_CALL_OK == error);

  // Client unrefs the call without starting recv_trailing_metadata.
  // This should trigger a cancellation.
  grpc_call_unref(c);

  // The send ops batch and the first recv ops batch will fail in most
  // fixtures but will pass in the proxy fixtures on some platforms.
  cqv.Expect(grpc_core::CqVerifier::tag(1), grpc_core::CqVerifier::AnyStatus());
  cqv.Expect(grpc_core::CqVerifier::tag(2), grpc_core::CqVerifier::AnyStatus());
  cqv.Expect(grpc_core::CqVerifier::tag(3), true);
  cqv.Verify();

  grpc_slice_unref(details);
  grpc_metadata_array_destroy(&initial_metadata_recv);
  grpc_metadata_array_destroy(&trailing_metadata_recv);
  grpc_byte_buffer_destroy(request_payload);
  grpc_byte_buffer_destroy(response_payload_recv);
}

void retry_cancel_after_first_attempt_starts(
    const CoreTestConfiguration& config) {
  GPR_ASSERT(config.feature_mask & FEATURE_MASK_SUPPORTS_CLIENT_CHANNEL);
  test_retry_cancel_after_first_attempt_starts(config);
}

void retry_cancel_after_first_attempt_starts_pre_init(void) {}
