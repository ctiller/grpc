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

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <grpc/grpc.h>
#include <grpc/impl/propagation_bits.h>
#include <grpc/slice.h>
#include <grpc/status.h>
#include <grpc/support/log.h>
#include <grpc/support/time.h>

#include "test/core/end2end/cq_verifier.h"
#include "test/core/end2end/end2end_tests.h"
#include "test/core/util/test_config.h"

static void* tag(intptr_t t) { return reinterpret_cast<void*>(t); }

static CoreTestFixture begin_test(CoreTestConfiguration config,
                                  const char* test_name,
                                  grpc_channel_args* client_args,
                                  grpc_channel_args* server_args) {
  CoreTestFixture f;
  gpr_log(GPR_INFO, "Running test: %s/%s", test_name, config.name);
  f = config.create_fixture(client_args, server_args);
  config.init_server(&f, server_args);
  config.init_client(&f, client_args);
  return f;
}

static gpr_timespec n_seconds_from_now(int n) {
  return grpc_timeout_seconds_to_deadline(n);
}

static gpr_timespec five_seconds_from_now(void) {
  return n_seconds_from_now(5);
}

static void drain_cq(grpc_completion_queue* cq) {
  grpc_event ev;
  do {
    ev = grpc_completion_queue_next(cq, five_seconds_from_now(), nullptr);
  } while (ev.type != GRPC_QUEUE_SHUTDOWN);
}

static void shutdown_server(CoreTestFixture* f) {
  if (!f->server()) return;
  grpc_server_shutdown_and_notify(f->server(), f->cq(), tag(1000));
  grpc_event ev;
  do {
    ev = grpc_completion_queue_next(
        f->cq(), grpc_timeout_seconds_to_deadline(5), nullptr);
  } while (ev.type != GRPC_OP_COMPLETE || ev.tag != tag(1000));
  grpc_server_destroy(f->server());
  f->server() = nullptr;
}

static void shutdown_client(CoreTestFixture* f) {
  if (!f->client()) return;
  grpc_channel_destroy(f->client());
  f->client() = nullptr;
}

static void end_test(CoreTestFixture* f) {
  shutdown_server(f);
  shutdown_client(f);

  grpc_completion_queue_shutdown(f->cq());
  drain_cq(f->cq());
  grpc_completion_queue_destroy(f->cq());
}

static void simple_request_body(CoreTestConfiguration /*config*/,
                                CoreTestFixture f, size_t num_ops) {
  grpc_call* c;
  grpc_core::CqVerifier cqv(f.cq);
  grpc_op ops[6];
  grpc_op* op;
  grpc_metadata_array initial_metadata_recv;
  grpc_metadata_array trailing_metadata_recv;
  grpc_status_code status;
  grpc_call_error error;
  grpc_slice details;

  gpr_log(GPR_DEBUG, "test with %" PRIuPTR " ops", num_ops);

  gpr_timespec deadline = gpr_inf_past(GPR_CLOCK_REALTIME);
  c = grpc_channel_create_call(f.client, nullptr, GRPC_PROPAGATE_DEFAULTS, f.cq,
                               grpc_slice_from_static_string("/foo"), nullptr,
                               deadline, nullptr);
  GPR_ASSERT(c);

  grpc_metadata_array_init(&initial_metadata_recv);
  grpc_metadata_array_init(&trailing_metadata_recv);

  memset(ops, 0, sizeof(ops));
  op = ops;
  op->op = GRPC_OP_RECV_STATUS_ON_CLIENT;
  op->data.recv_status_on_client.trailing_metadata = &trailing_metadata_recv;
  op->data.recv_status_on_client.status = &status;
  op->data.recv_status_on_client.status_details = &details;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  op->op = GRPC_OP_RECV_INITIAL_METADATA;
  op->data.recv_initial_metadata.recv_initial_metadata = &initial_metadata_recv;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  op->op = GRPC_OP_SEND_INITIAL_METADATA;
  op->data.send_initial_metadata.count = 0;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  op->op = GRPC_OP_SEND_CLOSE_FROM_CLIENT;
  op->flags = 0;
  op->reserved = nullptr;
  op++;
  GPR_ASSERT(num_ops <= (size_t)(op - ops));
  error = grpc_call_start_batch(c, ops, num_ops, tag(1), nullptr);
  GPR_ASSERT(GRPC_CALL_OK == error);

  cqv.Expect(tag(1), true);
  cqv.Verify();

  GPR_ASSERT(status == GRPC_STATUS_DEADLINE_EXCEEDED);

  grpc_slice_unref(details);
  grpc_metadata_array_destroy(&initial_metadata_recv);
  grpc_metadata_array_destroy(&trailing_metadata_recv);

  grpc_call_unref(c);
}

static void test_invoke_simple_request(CoreTestConfiguration config,
                                       size_t num_ops) {
  CoreTestFixture f;

  f = begin_test(config, "test_invoke_simple_request", nullptr, nullptr);
  simple_request_body(config, f, num_ops);
  end_test(&f);
  config.tear_down_data(&f);
}

void negative_deadline(CoreTestConfiguration config) {
  size_t i;
  for (i = 1; i <= 4; i++) {
    test_invoke_simple_request(config, i);
  }
}

void negative_deadline_pre_init(void) {}
