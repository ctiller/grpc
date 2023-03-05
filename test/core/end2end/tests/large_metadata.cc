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

#include <stdio.h>
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

// Request with a large amount of metadata.
static void test_request_with_large_metadata(
    const CoreTestConfiguration& config) {
  grpc_call* c;
  grpc_call* s;
  grpc_slice request_payload_slice =
      grpc_slice_from_copied_string("hello world");
  grpc_byte_buffer* request_payload =
      grpc_raw_byte_buffer_create(&request_payload_slice, 1);
  grpc_metadata meta;
  const size_t large_size = 64 * 1024;
  grpc_arg arg;
  arg.type = GRPC_ARG_INTEGER;
  arg.key = const_cast<char*>(GRPC_ARG_MAX_METADATA_SIZE);
  arg.value.integer = static_cast<int>(large_size) + 1024;
  grpc_channel_args args = {1, &arg};
  auto f = begin_test(config, "test_request_with_large_metadata", &args, &args);
  grpc_core::CqVerifier cqv(f->cq());
  grpc_op ops[6];
  grpc_op* op;
  grpc_metadata_array initial_metadata_recv;
  grpc_metadata_array trailing_metadata_recv;
  grpc_metadata_array request_metadata_recv;
  grpc_byte_buffer* request_payload_recv = nullptr;
  grpc_call_details call_details;
  grpc_status_code status;
  grpc_call_error error;
  grpc_slice details;
  int was_cancelled = 2;

  gpr_timespec deadline = grpc_timeout_seconds_to_deadline(5);
  c = grpc_channel_create_call(f->client(), nullptr, GRPC_PROPAGATE_DEFAULTS,
                               f->cq(), grpc_slice_from_static_string("/foo"),
                               nullptr, deadline, nullptr);
  GPR_ASSERT(c);

  meta.key = grpc_slice_from_static_string("key");
  meta.value = grpc_slice_malloc(large_size);
  memset(GRPC_SLICE_START_PTR(meta.value), 'a', large_size);

  grpc_metadata_array_init(&initial_metadata_recv);
  grpc_metadata_array_init(&trailing_metadata_recv);
  grpc_metadata_array_init(&request_metadata_recv);
  grpc_call_details_init(&call_details);

  memset(ops, 0, sizeof(ops));
  // Client: send request.
  op = ops;
  op->op = GRPC_OP_SEND_INITIAL_METADATA;
  op->data.send_initial_metadata.count = 1;
  op->data.send_initial_metadata.metadata = &meta;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  op->op = GRPC_OP_SEND_MESSAGE;
  op->data.send_message.send_message = request_payload;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  op->op = GRPC_OP_SEND_CLOSE_FROM_CLIENT;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  op->op = GRPC_OP_RECV_INITIAL_METADATA;
  op->data.recv_initial_metadata.recv_initial_metadata = &initial_metadata_recv;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  op->op = GRPC_OP_RECV_STATUS_ON_CLIENT;
  op->data.recv_status_on_client.trailing_metadata = &trailing_metadata_recv;
  op->data.recv_status_on_client.status = &status;
  op->data.recv_status_on_client.status_details = &details;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  error = grpc_call_start_batch(c, ops, static_cast<size_t>(op - ops),
                                grpc_core::CqVerifier::tag(1), nullptr);
  GPR_ASSERT(GRPC_CALL_OK == error);

  error = grpc_server_request_call(f->server(), &s, &call_details,
                                   &request_metadata_recv, f->cq(), f->cq(),
                                   grpc_core::CqVerifier::tag(101));
  GPR_ASSERT(GRPC_CALL_OK == error);

  cqv.Expect(grpc_core::CqVerifier::tag(101), true);
  cqv.Verify();

  memset(ops, 0, sizeof(ops));
  // Server: send initial metadata and receive request.
  op = ops;
  op->op = GRPC_OP_SEND_INITIAL_METADATA;
  op->data.send_initial_metadata.count = 0;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  op->op = GRPC_OP_RECV_MESSAGE;
  op->data.recv_message.recv_message = &request_payload_recv;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  error = grpc_call_start_batch(s, ops, static_cast<size_t>(op - ops),
                                grpc_core::CqVerifier::tag(102), nullptr);
  GPR_ASSERT(GRPC_CALL_OK == error);

  cqv.Expect(grpc_core::CqVerifier::tag(102), true);
  cqv.Verify();

  memset(ops, 0, sizeof(ops));
  // Server: receive close and send status.  This should trigger
  // completion of request on client.
  op = ops;
  op->op = GRPC_OP_RECV_CLOSE_ON_SERVER;
  op->data.recv_close_on_server.cancelled = &was_cancelled;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  op->op = GRPC_OP_SEND_STATUS_FROM_SERVER;
  op->data.send_status_from_server.trailing_metadata_count = 0;
  op->data.send_status_from_server.status = GRPC_STATUS_OK;
  grpc_slice status_details = grpc_slice_from_static_string("xyz");
  op->data.send_status_from_server.status_details = &status_details;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  error = grpc_call_start_batch(s, ops, static_cast<size_t>(op - ops),
                                grpc_core::CqVerifier::tag(103), nullptr);
  GPR_ASSERT(GRPC_CALL_OK == error);

  cqv.Expect(grpc_core::CqVerifier::tag(103), true);
  cqv.Expect(grpc_core::CqVerifier::tag(1), true);
  cqv.Verify();

  GPR_ASSERT(status == GRPC_STATUS_OK);
  GPR_ASSERT(0 == grpc_slice_str_cmp(details, "xyz"));
  GPR_ASSERT(0 == grpc_slice_str_cmp(call_details.method, "/foo"));
  GPR_ASSERT(was_cancelled == 0);
  GPR_ASSERT(byte_buffer_eq_string(request_payload_recv, "hello world"));
  GPR_ASSERT(contains_metadata_slices(&request_metadata_recv,
                                      grpc_slice_from_static_string("key"),
                                      meta.value));

  grpc_slice_unref(details);
  grpc_metadata_array_destroy(&initial_metadata_recv);
  grpc_metadata_array_destroy(&trailing_metadata_recv);
  grpc_metadata_array_destroy(&request_metadata_recv);
  grpc_call_details_destroy(&call_details);

  grpc_call_unref(c);
  grpc_call_unref(s);

  grpc_byte_buffer_destroy(request_payload);
  grpc_byte_buffer_destroy(request_payload_recv);

  grpc_slice_unref(meta.value);
}

// Server responds with metadata larger than what the client accepts.
static void test_request_with_bad_large_metadata_response(
    CoreTestConfiguration config) {
  grpc_arg arg;
  arg.type = GRPC_ARG_INTEGER;
  arg.key = const_cast<char*>(GRPC_ARG_MAX_METADATA_SIZE);
  arg.value.integer = 1024;
  grpc_channel_args args = {1, &arg};
  auto f = begin_test(config, "test_request_with_bad_large_metadata_response",
                      &args, &args);
  grpc_core::CqVerifier cqv(f->cq());

  for (int i = 0; i < 10; i++) {
    grpc_call* c;
    grpc_call* s;
    grpc_metadata meta;
    const size_t large_size = 64 * 1024;
    grpc_op ops[6];
    grpc_op* op;
    grpc_metadata_array initial_metadata_recv;
    grpc_metadata_array trailing_metadata_recv;
    grpc_metadata_array request_metadata_recv;
    grpc_call_details call_details;
    grpc_status_code status;
    grpc_call_error error;
    grpc_slice details;
    int was_cancelled = 2;

    gpr_timespec deadline = grpc_timeout_seconds_to_deadline(5);
    c = grpc_channel_create_call(f->client(), nullptr, GRPC_PROPAGATE_DEFAULTS,
                                 f->cq(), grpc_slice_from_static_string("/foo"),
                                 nullptr, deadline, nullptr);
    GPR_ASSERT(c);

    meta.key = grpc_slice_from_static_string("key");
    meta.value = grpc_slice_malloc(large_size);
    memset(GRPC_SLICE_START_PTR(meta.value), 'a', large_size);

    grpc_metadata_array_init(&initial_metadata_recv);
    grpc_metadata_array_init(&trailing_metadata_recv);
    grpc_metadata_array_init(&request_metadata_recv);
    grpc_call_details_init(&call_details);

    memset(ops, 0, sizeof(ops));
    // Client: send request.
    op = ops;
    op->op = GRPC_OP_SEND_INITIAL_METADATA;
    op->data.send_initial_metadata.count = 0;
    op->flags = 0;
    op->reserved = nullptr;
    op++;
    op->op = GRPC_OP_SEND_CLOSE_FROM_CLIENT;
    op->flags = 0;
    op->reserved = nullptr;
    op++;
    op->op = GRPC_OP_RECV_INITIAL_METADATA;
    op->data.recv_initial_metadata.recv_initial_metadata =
        &initial_metadata_recv;
    op->flags = 0;
    op->reserved = nullptr;
    op++;
    op->op = GRPC_OP_RECV_STATUS_ON_CLIENT;
    op->data.recv_status_on_client.trailing_metadata = &trailing_metadata_recv;
    op->data.recv_status_on_client.status = &status;
    op->data.recv_status_on_client.status_details = &details;
    op->flags = 0;
    op->reserved = nullptr;
    op++;
    error = grpc_call_start_batch(c, ops, static_cast<size_t>(op - ops),
                                  grpc_core::CqVerifier::tag(1), nullptr);
    GPR_ASSERT(GRPC_CALL_OK == error);

    error = grpc_server_request_call(f->server(), &s, &call_details,
                                     &request_metadata_recv, f->cq(), f->cq(),
                                     grpc_core::CqVerifier::tag(101));
    GPR_ASSERT(GRPC_CALL_OK == error);

    cqv.Expect(grpc_core::CqVerifier::tag(101), true);
    cqv.Verify();

    memset(ops, 0, sizeof(ops));
    // Server: send large initial metadata
    op = ops;
    op->op = GRPC_OP_SEND_INITIAL_METADATA;
    op->data.send_initial_metadata.count = 1;
    op->data.send_initial_metadata.metadata = &meta;
    op->flags = 0;
    op->reserved = nullptr;
    op++;
    op->op = GRPC_OP_RECV_CLOSE_ON_SERVER;
    op->data.recv_close_on_server.cancelled = &was_cancelled;
    op->flags = 0;
    op->reserved = nullptr;
    op++;
    op->op = GRPC_OP_SEND_STATUS_FROM_SERVER;
    op->data.send_status_from_server.trailing_metadata_count = 0;
    op->data.send_status_from_server.status = GRPC_STATUS_OK;
    grpc_slice status_details = grpc_slice_from_static_string("xyz");
    op->data.send_status_from_server.status_details = &status_details;
    op->flags = 0;
    op->reserved = nullptr;
    op++;
    error = grpc_call_start_batch(s, ops, static_cast<size_t>(op - ops),
                                  grpc_core::CqVerifier::tag(102), nullptr);
    GPR_ASSERT(GRPC_CALL_OK == error);
    cqv.Expect(grpc_core::CqVerifier::tag(102), true);
    cqv.Expect(grpc_core::CqVerifier::tag(1), true);
    cqv.Verify();

    GPR_ASSERT(status == GRPC_STATUS_RESOURCE_EXHAUSTED);
    const char* expected_error = "received initial metadata size exceeds limit";
    grpc_slice actual_error =
        grpc_slice_split_head(&details, strlen(expected_error));
    GPR_ASSERT(0 == grpc_slice_str_cmp(actual_error, expected_error));
    GPR_ASSERT(0 == grpc_slice_str_cmp(call_details.method, "/foo"));

    grpc_slice_unref(actual_error);
    grpc_slice_unref(details);
    grpc_metadata_array_destroy(&initial_metadata_recv);
    grpc_metadata_array_destroy(&trailing_metadata_recv);
    grpc_metadata_array_destroy(&request_metadata_recv);
    grpc_call_details_destroy(&call_details);

    grpc_call_unref(c);
    grpc_call_unref(s);

    grpc_slice_unref(meta.value);
  }
}

void large_metadata(const CoreTestConfiguration& config) {
  test_request_with_large_metadata(config);
  // TODO(yashykt): Maybe add checks for metadata size in inproc transport too.
  if (strcmp(config.name, "inproc") != 0) {
    test_request_with_bad_large_metadata_response(config);
  }
}

void large_metadata_pre_init(void) {}
