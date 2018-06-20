/*
 *
 * Copyright 2018 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <grpc/support/port_platform.h>

#include "src/cpp/ext/filters/census/server_filter.h"

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "opencensus/stats/stats.h"
#include "src/core/lib/surface/call.h"
#include "src/cpp/ext/filters/census/grpc_plugin.h"
#include "src/cpp/ext/filters/census/measures.h"

namespace grpc {

constexpr uint32_t CensusServerCallData::kMaxServerStatsLen;

namespace {

// server metadata elements
struct ServerMetadataElements {
  grpc_slice path;
  grpc_slice tracing_slice;
  grpc_slice census_proto;
};

void FilterInitialMetadata(grpc_metadata_batch* b,
                           ServerMetadataElements* sml) {
  if (b->idx.named.path != nullptr) {
    sml->path = grpc_slice_ref_internal(GRPC_MDVALUE(b->idx.named.path->md));
  }
  if (b->idx.named.grpc_trace_bin != nullptr) {
    sml->tracing_slice =
        grpc_slice_ref_internal(GRPC_MDVALUE(b->idx.named.grpc_trace_bin->md));
    grpc_metadata_batch_remove(b, b->idx.named.grpc_trace_bin);
  }
  if (b->idx.named.grpc_tags_bin != nullptr) {
    sml->census_proto =
        grpc_slice_ref_internal(GRPC_MDVALUE(b->idx.named.grpc_tags_bin->md));
    grpc_metadata_batch_remove(b, b->idx.named.grpc_tags_bin);
  }
}

}  // namespace

void CensusServerCallData::OnDoneRecvInitialMetadata(
    grpc_transport_stream_recv_op_batch* batch) {
  ServerMetadataElements sml;
  sml.path = grpc_empty_slice();
  sml.tracing_slice = grpc_empty_slice();
  sml.census_proto = grpc_empty_slice();
  FilterInitialMetadata(
      batch->payload->recv_initial_metadata.recv_initial_metadata, &sml);
  path_ = grpc_slice_ref_internal(sml.path);
  method_ = GetMethod(&path_);
  qualified_method_ = StrCat("Recv.", method_);
  const char* tracing_str =
      GRPC_SLICE_IS_EMPTY(sml.tracing_slice)
          ? ""
          : reinterpret_cast<const char*>(
                GRPC_SLICE_START_PTR(sml.tracing_slice));
  size_t tracing_str_len = GRPC_SLICE_IS_EMPTY(sml.tracing_slice)
                               ? 0
                               : GRPC_SLICE_LENGTH(sml.tracing_slice);
  const char* census_str = GRPC_SLICE_IS_EMPTY(sml.census_proto)
                               ? ""
                               : reinterpret_cast<const char*>(
                                     GRPC_SLICE_START_PTR(sml.census_proto));
  size_t census_str_len = GRPC_SLICE_IS_EMPTY(sml.census_proto)
                              ? 0
                              : GRPC_SLICE_LENGTH(sml.census_proto);

  GenerateServerContext(absl::string_view(tracing_str, tracing_str_len),
                        absl::string_view(census_str, census_str_len),
                        /*primary_role*/ "", qualified_method_,
                        &context_);

  grpc_slice_unref_internal(sml.tracing_slice);
  grpc_slice_unref_internal(sml.census_proto);
  grpc_slice_unref_internal(sml.path);
  grpc_census_call_set_context(
      gc_, reinterpret_cast<census_context*>(&context_));
}

void CensusServerCallData::StartTransportStreamOpBatch(
    grpc_call_element* elem, TransportStreamOpBatch* op) {
  if (op->send_message() != nullptr) {
    ++sent_message_count_;
  }
  // We need to record the time when the trailing metadata was sent to mark the
  // completeness of the request.
  if (op->send_trailing_metadata() != nullptr) {
    elapsed_time_ = absl::Now() - start_time_;
    size_t len = ServerStatsSerialize(absl::ToInt64Nanoseconds(elapsed_time_),
                                      stats_buf_, kMaxServerStatsLen);
    if (len > 0) {
      GRPC_LOG_IF_ERROR(
          "census grpc_filter",
          grpc_metadata_batch_add_tail(
              op->send_trailing_metadata()->batch(), &census_bin_,
              grpc_mdelem_from_slices(
                  GRPC_MDSTR_GRPC_SERVER_STATS_BIN,
                  grpc_slice_from_copied_buffer(stats_buf_, len))));
    }
  }
  // Call next op.
  grpc_call_next_op(elem, op->op());
}

void CensusServerCallData::StartTransportStreamRecvOpBatch(
    grpc_call_element* elem, grpc_transport_stream_recv_op_batch* batch,
    grpc_error* error) {
  if (batch->recv_initial_metadata && error == GRPC_ERROR_NONE) {
    OnDoneRecvInitialMetadata(batch);
  }
  if (batch->recv_message) {
    if (*batch->payload->recv_message.recv_message != nullptr) {
      ++recv_message_count_;
    }
  }
  grpc_call_prev_filter_recv_op_batch(elem, batch, error);
}

grpc_error* CensusServerCallData::Init(grpc_call_element* elem,
                                       const grpc_call_element_args* args) {
  start_time_ = absl::Now();
  gc_ =
      grpc_call_from_top_element(grpc_call_stack_element(args->call_stack, 0));
  auth_context_ = grpc_call_auth_context(gc_);
  return GRPC_ERROR_NONE;
}

void CensusServerCallData::Destroy(grpc_call_element* elem,
                                   const grpc_call_final_info* final_info,
                                   grpc_closure* then_call_closure) {
  const uint64_t request_size = GetOutgoingDataSize(final_info);
  const uint64_t response_size = GetIncomingDataSize(final_info);
  double elapsed_time_ms = absl::ToDoubleMilliseconds(elapsed_time_);
  grpc_auth_context_release(auth_context_);
  ::opencensus::stats::Record(
      {{RpcServerSentBytesPerRpc(), static_cast<double>(response_size)},
       {RpcServerReceivedBytesPerRpc(), static_cast<double>(request_size)},
       {RpcServerServerLatency(), elapsed_time_ms},
       {RpcServerSentMessagesPerRpc(), sent_message_count_},
       {RpcServerReceivedMessagesPerRpc(), recv_message_count_}},
      {{ServerMethodTagKey(), method_},
       {ServerStatusTagKey(), StatusCodeToString(final_info->final_status)}});
  grpc_slice_unref_internal(path_);
  context_.EndSpan();
}

}  // namespace grpc
