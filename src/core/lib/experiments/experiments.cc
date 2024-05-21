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

// Auto generated by tools/codegen/core/gen_experiments.py

#include "src/core/lib/experiments/experiments.h"

#include <stdint.h>

#include <grpc/support/port_platform.h>

#ifndef GRPC_EXPERIMENTS_ARE_FINAL

#if defined(GRPC_CFSTREAM)
namespace {
const char* const description_call_status_override_on_cancellation =
    "Avoid overriding call status of successfully finished calls if it races "
    "with cancellation.";
const char* const additional_constraints_call_status_override_on_cancellation =
    "{}";
const char* const description_call_v3 = "Promise-based call version 3.";
const char* const additional_constraints_call_v3 = "{}";
const char* const description_canary_client_privacy =
    "If set, canary client privacy";
const char* const additional_constraints_canary_client_privacy = "{}";
const char* const description_client_privacy = "If set, client privacy";
const char* const additional_constraints_client_privacy = "{}";
const char* const description_event_engine_client =
    "Use EventEngine clients instead of iomgr's grpc_tcp_client";
const char* const additional_constraints_event_engine_client = "{}";
const char* const description_event_engine_dns =
    "If set, use EventEngine DNSResolver for client channel resolution";
const char* const additional_constraints_event_engine_dns = "{}";
const char* const description_event_engine_listener =
    "Use EventEngine listeners instead of iomgr's grpc_tcp_server";
const char* const additional_constraints_event_engine_listener = "{}";
const char* const description_free_large_allocator =
    "If set, return all free bytes from a \042big\042 allocator";
const char* const additional_constraints_free_large_allocator = "{}";
const char* const description_http2_stats_fix =
    "Fix on HTTP2 outgoing data stats reporting";
const char* const additional_constraints_http2_stats_fix = "{}";
const char* const description_keepalive_fix =
    "Allows overriding keepalive_permit_without_calls. Refer "
    "https://github.com/grpc/grpc/pull/33428 for more information.";
const char* const additional_constraints_keepalive_fix = "{}";
const char* const description_keepalive_server_fix =
    "Allows overriding keepalive_permit_without_calls for servers. Refer "
    "https://github.com/grpc/grpc/pull/33917 for more information.";
const char* const additional_constraints_keepalive_server_fix = "{}";
const char* const description_monitoring_experiment =
    "Placeholder experiment to prove/disprove our monitoring is working";
const char* const additional_constraints_monitoring_experiment = "{}";
const char* const description_multiping =
    "Allow more than one ping to be in flight at a time by default.";
const char* const additional_constraints_multiping = "{}";
const char* const description_peer_state_based_framing =
    "If set, the max sizes of frames sent to lower layers is controlled based "
    "on the peer's memory pressure which is reflected in its max http2 frame "
    "size.";
const char* const additional_constraints_peer_state_based_framing = "{}";
const char* const description_pick_first_new =
    "New pick_first impl with memory reduction.";
const char* const additional_constraints_pick_first_new = "{}";
const char* const description_promise_based_client_call =
    "If set, use the new gRPC promise based call code when it's appropriate "
    "(ie when all filters in a stack are promise based)";
const char* const additional_constraints_promise_based_client_call = "{}";
const uint8_t required_experiments_promise_based_client_call[] = {
    static_cast<uint8_t>(grpc_core::kExperimentIdEventEngineClient),
    static_cast<uint8_t>(grpc_core::kExperimentIdEventEngineListener)};
const char* const description_chaotic_good =
    "If set, enable the chaotic good load transport (this is mostly here for "
    "testing)";
const char* const additional_constraints_chaotic_good = "{}";
const uint8_t required_experiments_chaotic_good[] = {
    static_cast<uint8_t>(grpc_core::kExperimentIdPromiseBasedClientCall)};
const char* const description_promise_based_inproc_transport =
    "Use promises for the in-process transport.";
const char* const additional_constraints_promise_based_inproc_transport = "{}";
const uint8_t required_experiments_promise_based_inproc_transport[] = {
    static_cast<uint8_t>(grpc_core::kExperimentIdPromiseBasedClientCall)};
const char* const description_rstpit =
    "On RST_STREAM on a server, reduce MAX_CONCURRENT_STREAMS for a short "
    "duration";
const char* const additional_constraints_rstpit = "{}";
const char* const description_schedule_cancellation_over_write =
    "Allow cancellation op to be scheduled over a write";
const char* const additional_constraints_schedule_cancellation_over_write =
    "{}";
const char* const description_server_privacy = "If set, server privacy";
const char* const additional_constraints_server_privacy = "{}";
const char* const description_tcp_frame_size_tuning =
    "If set, enables TCP to use RPC size estimation made by higher layers. TCP "
    "would not indicate completion of a read operation until a specified "
    "number of bytes have been read over the socket. Buffers are also "
    "allocated according to estimated RPC sizes.";
const char* const additional_constraints_tcp_frame_size_tuning = "{}";
const char* const description_tcp_rcv_lowat =
    "Use SO_RCVLOWAT to avoid wakeups on the read path.";
const char* const additional_constraints_tcp_rcv_lowat = "{}";
const char* const description_trace_record_callops =
    "Enables tracing of call batch initiation and completion.";
const char* const additional_constraints_trace_record_callops = "{}";
const char* const description_unconstrained_max_quota_buffer_size =
    "Discard the cap on the max free pool size for one memory allocator";
const char* const additional_constraints_unconstrained_max_quota_buffer_size =
    "{}";
const char* const description_work_serializer_clears_time_cache =
    "Have the work serializer clear the time cache when it dispatches work.";
const char* const additional_constraints_work_serializer_clears_time_cache =
    "{}";
const char* const description_work_serializer_dispatch =
    "Have the work serializer dispatch work to event engine for every "
    "callback, instead of running things inline in the first thread that "
    "successfully enqueues work.";
const char* const additional_constraints_work_serializer_dispatch = "{}";
const uint8_t required_experiments_work_serializer_dispatch[] = {
    static_cast<uint8_t>(grpc_core::kExperimentIdEventEngineClient)};
}  // namespace

namespace grpc_core {

const ExperimentMetadata g_experiment_metadata[] = {
    {"call_status_override_on_cancellation",
     description_call_status_override_on_cancellation,
     additional_constraints_call_status_override_on_cancellation, nullptr, 0,
     true, true},
    {"call_v3", description_call_v3, additional_constraints_call_v3, nullptr, 0,
     false, true},
    {"canary_client_privacy", description_canary_client_privacy,
     additional_constraints_canary_client_privacy, nullptr, 0, false, false},
    {"client_privacy", description_client_privacy,
     additional_constraints_client_privacy, nullptr, 0, false, false},
    {"event_engine_client", description_event_engine_client,
     additional_constraints_event_engine_client, nullptr, 0, false, true},
    {"event_engine_dns", description_event_engine_dns,
     additional_constraints_event_engine_dns, nullptr, 0, false, false},
    {"event_engine_listener", description_event_engine_listener,
     additional_constraints_event_engine_listener, nullptr, 0, false, true},
    {"free_large_allocator", description_free_large_allocator,
     additional_constraints_free_large_allocator, nullptr, 0, false, true},
    {"http2_stats_fix", description_http2_stats_fix,
     additional_constraints_http2_stats_fix, nullptr, 0, true, true},
    {"keepalive_fix", description_keepalive_fix,
     additional_constraints_keepalive_fix, nullptr, 0, false, false},
    {"keepalive_server_fix", description_keepalive_server_fix,
     additional_constraints_keepalive_server_fix, nullptr, 0, false, false},
    {"monitoring_experiment", description_monitoring_experiment,
     additional_constraints_monitoring_experiment, nullptr, 0, true, true},
    {"multiping", description_multiping, additional_constraints_multiping,
     nullptr, 0, false, true},
    {"peer_state_based_framing", description_peer_state_based_framing,
     additional_constraints_peer_state_based_framing, nullptr, 0, false, true},
    {"pick_first_new", description_pick_first_new,
     additional_constraints_pick_first_new, nullptr, 0, true, true},
    {"promise_based_client_call", description_promise_based_client_call,
     additional_constraints_promise_based_client_call,
     required_experiments_promise_based_client_call, 2, false, true},
    {"chaotic_good", description_chaotic_good,
     additional_constraints_chaotic_good, required_experiments_chaotic_good, 1,
     false, true},
    {"promise_based_inproc_transport",
     description_promise_based_inproc_transport,
     additional_constraints_promise_based_inproc_transport,
     required_experiments_promise_based_inproc_transport, 1, false, false},
    {"rstpit", description_rstpit, additional_constraints_rstpit, nullptr, 0,
     false, true},
    {"schedule_cancellation_over_write",
     description_schedule_cancellation_over_write,
     additional_constraints_schedule_cancellation_over_write, nullptr, 0, false,
     true},
    {"server_privacy", description_server_privacy,
     additional_constraints_server_privacy, nullptr, 0, false, false},
    {"tcp_frame_size_tuning", description_tcp_frame_size_tuning,
     additional_constraints_tcp_frame_size_tuning, nullptr, 0, false, true},
    {"tcp_rcv_lowat", description_tcp_rcv_lowat,
     additional_constraints_tcp_rcv_lowat, nullptr, 0, false, true},
    {"trace_record_callops", description_trace_record_callops,
     additional_constraints_trace_record_callops, nullptr, 0, true, true},
    {"unconstrained_max_quota_buffer_size",
     description_unconstrained_max_quota_buffer_size,
     additional_constraints_unconstrained_max_quota_buffer_size, nullptr, 0,
     false, true},
    {"work_serializer_clears_time_cache",
     description_work_serializer_clears_time_cache,
     additional_constraints_work_serializer_clears_time_cache, nullptr, 0, true,
     true},
    {"work_serializer_dispatch", description_work_serializer_dispatch,
     additional_constraints_work_serializer_dispatch,
     required_experiments_work_serializer_dispatch, 1, false, true},
};

}  // namespace grpc_core

#elif defined(GPR_WINDOWS)
namespace {
const char* const description_call_status_override_on_cancellation =
    "Avoid overriding call status of successfully finished calls if it races "
    "with cancellation.";
const char* const additional_constraints_call_status_override_on_cancellation =
    "{}";
const char* const description_call_v3 = "Promise-based call version 3.";
const char* const additional_constraints_call_v3 = "{}";
const char* const description_canary_client_privacy =
    "If set, canary client privacy";
const char* const additional_constraints_canary_client_privacy = "{}";
const char* const description_client_privacy = "If set, client privacy";
const char* const additional_constraints_client_privacy = "{}";
const char* const description_event_engine_client =
    "Use EventEngine clients instead of iomgr's grpc_tcp_client";
const char* const additional_constraints_event_engine_client = "{}";
const char* const description_event_engine_dns =
    "If set, use EventEngine DNSResolver for client channel resolution";
const char* const additional_constraints_event_engine_dns = "{}";
const char* const description_event_engine_listener =
    "Use EventEngine listeners instead of iomgr's grpc_tcp_server";
const char* const additional_constraints_event_engine_listener = "{}";
const char* const description_free_large_allocator =
    "If set, return all free bytes from a \042big\042 allocator";
const char* const additional_constraints_free_large_allocator = "{}";
const char* const description_http2_stats_fix =
    "Fix on HTTP2 outgoing data stats reporting";
const char* const additional_constraints_http2_stats_fix = "{}";
const char* const description_keepalive_fix =
    "Allows overriding keepalive_permit_without_calls. Refer "
    "https://github.com/grpc/grpc/pull/33428 for more information.";
const char* const additional_constraints_keepalive_fix = "{}";
const char* const description_keepalive_server_fix =
    "Allows overriding keepalive_permit_without_calls for servers. Refer "
    "https://github.com/grpc/grpc/pull/33917 for more information.";
const char* const additional_constraints_keepalive_server_fix = "{}";
const char* const description_monitoring_experiment =
    "Placeholder experiment to prove/disprove our monitoring is working";
const char* const additional_constraints_monitoring_experiment = "{}";
const char* const description_multiping =
    "Allow more than one ping to be in flight at a time by default.";
const char* const additional_constraints_multiping = "{}";
const char* const description_peer_state_based_framing =
    "If set, the max sizes of frames sent to lower layers is controlled based "
    "on the peer's memory pressure which is reflected in its max http2 frame "
    "size.";
const char* const additional_constraints_peer_state_based_framing = "{}";
const char* const description_pick_first_new =
    "New pick_first impl with memory reduction.";
const char* const additional_constraints_pick_first_new = "{}";
const char* const description_promise_based_client_call =
    "If set, use the new gRPC promise based call code when it's appropriate "
    "(ie when all filters in a stack are promise based)";
const char* const additional_constraints_promise_based_client_call = "{}";
const uint8_t required_experiments_promise_based_client_call[] = {
    static_cast<uint8_t>(grpc_core::kExperimentIdEventEngineClient),
    static_cast<uint8_t>(grpc_core::kExperimentIdEventEngineListener)};
const char* const description_chaotic_good =
    "If set, enable the chaotic good load transport (this is mostly here for "
    "testing)";
const char* const additional_constraints_chaotic_good = "{}";
const uint8_t required_experiments_chaotic_good[] = {
    static_cast<uint8_t>(grpc_core::kExperimentIdPromiseBasedClientCall)};
const char* const description_promise_based_inproc_transport =
    "Use promises for the in-process transport.";
const char* const additional_constraints_promise_based_inproc_transport = "{}";
const uint8_t required_experiments_promise_based_inproc_transport[] = {
    static_cast<uint8_t>(grpc_core::kExperimentIdPromiseBasedClientCall)};
const char* const description_rstpit =
    "On RST_STREAM on a server, reduce MAX_CONCURRENT_STREAMS for a short "
    "duration";
const char* const additional_constraints_rstpit = "{}";
const char* const description_schedule_cancellation_over_write =
    "Allow cancellation op to be scheduled over a write";
const char* const additional_constraints_schedule_cancellation_over_write =
    "{}";
const char* const description_server_privacy = "If set, server privacy";
const char* const additional_constraints_server_privacy = "{}";
const char* const description_tcp_frame_size_tuning =
    "If set, enables TCP to use RPC size estimation made by higher layers. TCP "
    "would not indicate completion of a read operation until a specified "
    "number of bytes have been read over the socket. Buffers are also "
    "allocated according to estimated RPC sizes.";
const char* const additional_constraints_tcp_frame_size_tuning = "{}";
const char* const description_tcp_rcv_lowat =
    "Use SO_RCVLOWAT to avoid wakeups on the read path.";
const char* const additional_constraints_tcp_rcv_lowat = "{}";
const char* const description_trace_record_callops =
    "Enables tracing of call batch initiation and completion.";
const char* const additional_constraints_trace_record_callops = "{}";
const char* const description_unconstrained_max_quota_buffer_size =
    "Discard the cap on the max free pool size for one memory allocator";
const char* const additional_constraints_unconstrained_max_quota_buffer_size =
    "{}";
const char* const description_work_serializer_clears_time_cache =
    "Have the work serializer clear the time cache when it dispatches work.";
const char* const additional_constraints_work_serializer_clears_time_cache =
    "{}";
const char* const description_work_serializer_dispatch =
    "Have the work serializer dispatch work to event engine for every "
    "callback, instead of running things inline in the first thread that "
    "successfully enqueues work.";
const char* const additional_constraints_work_serializer_dispatch = "{}";
const uint8_t required_experiments_work_serializer_dispatch[] = {
    static_cast<uint8_t>(grpc_core::kExperimentIdEventEngineClient)};
}  // namespace

namespace grpc_core {

const ExperimentMetadata g_experiment_metadata[] = {
    {"call_status_override_on_cancellation",
     description_call_status_override_on_cancellation,
     additional_constraints_call_status_override_on_cancellation, nullptr, 0,
     true, true},
    {"call_v3", description_call_v3, additional_constraints_call_v3, nullptr, 0,
     false, true},
    {"canary_client_privacy", description_canary_client_privacy,
     additional_constraints_canary_client_privacy, nullptr, 0, false, false},
    {"client_privacy", description_client_privacy,
     additional_constraints_client_privacy, nullptr, 0, false, false},
    {"event_engine_client", description_event_engine_client,
     additional_constraints_event_engine_client, nullptr, 0, true, true},
    {"event_engine_dns", description_event_engine_dns,
     additional_constraints_event_engine_dns, nullptr, 0, true, false},
    {"event_engine_listener", description_event_engine_listener,
     additional_constraints_event_engine_listener, nullptr, 0, true, true},
    {"free_large_allocator", description_free_large_allocator,
     additional_constraints_free_large_allocator, nullptr, 0, false, true},
    {"http2_stats_fix", description_http2_stats_fix,
     additional_constraints_http2_stats_fix, nullptr, 0, true, true},
    {"keepalive_fix", description_keepalive_fix,
     additional_constraints_keepalive_fix, nullptr, 0, false, false},
    {"keepalive_server_fix", description_keepalive_server_fix,
     additional_constraints_keepalive_server_fix, nullptr, 0, false, false},
    {"monitoring_experiment", description_monitoring_experiment,
     additional_constraints_monitoring_experiment, nullptr, 0, true, true},
    {"multiping", description_multiping, additional_constraints_multiping,
     nullptr, 0, false, true},
    {"peer_state_based_framing", description_peer_state_based_framing,
     additional_constraints_peer_state_based_framing, nullptr, 0, false, true},
    {"pick_first_new", description_pick_first_new,
     additional_constraints_pick_first_new, nullptr, 0, true, true},
    {"promise_based_client_call", description_promise_based_client_call,
     additional_constraints_promise_based_client_call,
     required_experiments_promise_based_client_call, 2, false, true},
    {"chaotic_good", description_chaotic_good,
     additional_constraints_chaotic_good, required_experiments_chaotic_good, 1,
     false, true},
    {"promise_based_inproc_transport",
     description_promise_based_inproc_transport,
     additional_constraints_promise_based_inproc_transport,
     required_experiments_promise_based_inproc_transport, 1, false, false},
    {"rstpit", description_rstpit, additional_constraints_rstpit, nullptr, 0,
     false, true},
    {"schedule_cancellation_over_write",
     description_schedule_cancellation_over_write,
     additional_constraints_schedule_cancellation_over_write, nullptr, 0, false,
     true},
    {"server_privacy", description_server_privacy,
     additional_constraints_server_privacy, nullptr, 0, false, false},
    {"tcp_frame_size_tuning", description_tcp_frame_size_tuning,
     additional_constraints_tcp_frame_size_tuning, nullptr, 0, false, true},
    {"tcp_rcv_lowat", description_tcp_rcv_lowat,
     additional_constraints_tcp_rcv_lowat, nullptr, 0, false, true},
    {"trace_record_callops", description_trace_record_callops,
     additional_constraints_trace_record_callops, nullptr, 0, true, true},
    {"unconstrained_max_quota_buffer_size",
     description_unconstrained_max_quota_buffer_size,
     additional_constraints_unconstrained_max_quota_buffer_size, nullptr, 0,
     false, true},
    {"work_serializer_clears_time_cache",
     description_work_serializer_clears_time_cache,
     additional_constraints_work_serializer_clears_time_cache, nullptr, 0, true,
     true},
    {"work_serializer_dispatch", description_work_serializer_dispatch,
     additional_constraints_work_serializer_dispatch,
     required_experiments_work_serializer_dispatch, 1, false, true},
};

}  // namespace grpc_core

#else
namespace {
const char* const description_call_status_override_on_cancellation =
    "Avoid overriding call status of successfully finished calls if it races "
    "with cancellation.";
const char* const additional_constraints_call_status_override_on_cancellation =
    "{}";
const char* const description_call_v3 = "Promise-based call version 3.";
const char* const additional_constraints_call_v3 = "{}";
const char* const description_canary_client_privacy =
    "If set, canary client privacy";
const char* const additional_constraints_canary_client_privacy = "{}";
const char* const description_client_privacy = "If set, client privacy";
const char* const additional_constraints_client_privacy = "{}";
const char* const description_event_engine_client =
    "Use EventEngine clients instead of iomgr's grpc_tcp_client";
const char* const additional_constraints_event_engine_client = "{}";
const char* const description_event_engine_dns =
    "If set, use EventEngine DNSResolver for client channel resolution";
const char* const additional_constraints_event_engine_dns = "{}";
const char* const description_event_engine_listener =
    "Use EventEngine listeners instead of iomgr's grpc_tcp_server";
const char* const additional_constraints_event_engine_listener = "{}";
const char* const description_free_large_allocator =
    "If set, return all free bytes from a \042big\042 allocator";
const char* const additional_constraints_free_large_allocator = "{}";
const char* const description_http2_stats_fix =
    "Fix on HTTP2 outgoing data stats reporting";
const char* const additional_constraints_http2_stats_fix = "{}";
const char* const description_keepalive_fix =
    "Allows overriding keepalive_permit_without_calls. Refer "
    "https://github.com/grpc/grpc/pull/33428 for more information.";
const char* const additional_constraints_keepalive_fix = "{}";
const char* const description_keepalive_server_fix =
    "Allows overriding keepalive_permit_without_calls for servers. Refer "
    "https://github.com/grpc/grpc/pull/33917 for more information.";
const char* const additional_constraints_keepalive_server_fix = "{}";
const char* const description_monitoring_experiment =
    "Placeholder experiment to prove/disprove our monitoring is working";
const char* const additional_constraints_monitoring_experiment = "{}";
const char* const description_multiping =
    "Allow more than one ping to be in flight at a time by default.";
const char* const additional_constraints_multiping = "{}";
const char* const description_peer_state_based_framing =
    "If set, the max sizes of frames sent to lower layers is controlled based "
    "on the peer's memory pressure which is reflected in its max http2 frame "
    "size.";
const char* const additional_constraints_peer_state_based_framing = "{}";
const char* const description_pick_first_new =
    "New pick_first impl with memory reduction.";
const char* const additional_constraints_pick_first_new = "{}";
const char* const description_promise_based_client_call =
    "If set, use the new gRPC promise based call code when it's appropriate "
    "(ie when all filters in a stack are promise based)";
const char* const additional_constraints_promise_based_client_call = "{}";
const uint8_t required_experiments_promise_based_client_call[] = {
    static_cast<uint8_t>(grpc_core::kExperimentIdEventEngineClient),
    static_cast<uint8_t>(grpc_core::kExperimentIdEventEngineListener)};
const char* const description_chaotic_good =
    "If set, enable the chaotic good load transport (this is mostly here for "
    "testing)";
const char* const additional_constraints_chaotic_good = "{}";
const uint8_t required_experiments_chaotic_good[] = {
    static_cast<uint8_t>(grpc_core::kExperimentIdPromiseBasedClientCall)};
const char* const description_promise_based_inproc_transport =
    "Use promises for the in-process transport.";
const char* const additional_constraints_promise_based_inproc_transport = "{}";
const uint8_t required_experiments_promise_based_inproc_transport[] = {
    static_cast<uint8_t>(grpc_core::kExperimentIdPromiseBasedClientCall)};
const char* const description_rstpit =
    "On RST_STREAM on a server, reduce MAX_CONCURRENT_STREAMS for a short "
    "duration";
const char* const additional_constraints_rstpit = "{}";
const char* const description_schedule_cancellation_over_write =
    "Allow cancellation op to be scheduled over a write";
const char* const additional_constraints_schedule_cancellation_over_write =
    "{}";
const char* const description_server_privacy = "If set, server privacy";
const char* const additional_constraints_server_privacy = "{}";
const char* const description_tcp_frame_size_tuning =
    "If set, enables TCP to use RPC size estimation made by higher layers. TCP "
    "would not indicate completion of a read operation until a specified "
    "number of bytes have been read over the socket. Buffers are also "
    "allocated according to estimated RPC sizes.";
const char* const additional_constraints_tcp_frame_size_tuning = "{}";
const char* const description_tcp_rcv_lowat =
    "Use SO_RCVLOWAT to avoid wakeups on the read path.";
const char* const additional_constraints_tcp_rcv_lowat = "{}";
const char* const description_trace_record_callops =
    "Enables tracing of call batch initiation and completion.";
const char* const additional_constraints_trace_record_callops = "{}";
const char* const description_unconstrained_max_quota_buffer_size =
    "Discard the cap on the max free pool size for one memory allocator";
const char* const additional_constraints_unconstrained_max_quota_buffer_size =
    "{}";
const char* const description_work_serializer_clears_time_cache =
    "Have the work serializer clear the time cache when it dispatches work.";
const char* const additional_constraints_work_serializer_clears_time_cache =
    "{}";
const char* const description_work_serializer_dispatch =
    "Have the work serializer dispatch work to event engine for every "
    "callback, instead of running things inline in the first thread that "
    "successfully enqueues work.";
const char* const additional_constraints_work_serializer_dispatch = "{}";
const uint8_t required_experiments_work_serializer_dispatch[] = {
    static_cast<uint8_t>(grpc_core::kExperimentIdEventEngineClient)};
}  // namespace

namespace grpc_core {

const ExperimentMetadata g_experiment_metadata[] = {
    {"call_status_override_on_cancellation",
     description_call_status_override_on_cancellation,
     additional_constraints_call_status_override_on_cancellation, nullptr, 0,
     true, true},
    {"call_v3", description_call_v3, additional_constraints_call_v3, nullptr, 0,
     false, true},
    {"canary_client_privacy", description_canary_client_privacy,
     additional_constraints_canary_client_privacy, nullptr, 0, false, false},
    {"client_privacy", description_client_privacy,
     additional_constraints_client_privacy, nullptr, 0, false, false},
    {"event_engine_client", description_event_engine_client,
     additional_constraints_event_engine_client, nullptr, 0, false, true},
    {"event_engine_dns", description_event_engine_dns,
     additional_constraints_event_engine_dns, nullptr, 0, true, false},
    {"event_engine_listener", description_event_engine_listener,
     additional_constraints_event_engine_listener, nullptr, 0, true, true},
    {"free_large_allocator", description_free_large_allocator,
     additional_constraints_free_large_allocator, nullptr, 0, false, true},
    {"http2_stats_fix", description_http2_stats_fix,
     additional_constraints_http2_stats_fix, nullptr, 0, true, true},
    {"keepalive_fix", description_keepalive_fix,
     additional_constraints_keepalive_fix, nullptr, 0, false, false},
    {"keepalive_server_fix", description_keepalive_server_fix,
     additional_constraints_keepalive_server_fix, nullptr, 0, false, false},
    {"monitoring_experiment", description_monitoring_experiment,
     additional_constraints_monitoring_experiment, nullptr, 0, true, true},
    {"multiping", description_multiping, additional_constraints_multiping,
     nullptr, 0, false, true},
    {"peer_state_based_framing", description_peer_state_based_framing,
     additional_constraints_peer_state_based_framing, nullptr, 0, false, true},
    {"pick_first_new", description_pick_first_new,
     additional_constraints_pick_first_new, nullptr, 0, true, true},
    {"promise_based_client_call", description_promise_based_client_call,
     additional_constraints_promise_based_client_call,
     required_experiments_promise_based_client_call, 2, false, true},
    {"chaotic_good", description_chaotic_good,
     additional_constraints_chaotic_good, required_experiments_chaotic_good, 1,
     false, true},
    {"promise_based_inproc_transport",
     description_promise_based_inproc_transport,
     additional_constraints_promise_based_inproc_transport,
     required_experiments_promise_based_inproc_transport, 1, false, false},
    {"rstpit", description_rstpit, additional_constraints_rstpit, nullptr, 0,
     false, true},
    {"schedule_cancellation_over_write",
     description_schedule_cancellation_over_write,
     additional_constraints_schedule_cancellation_over_write, nullptr, 0, false,
     true},
    {"server_privacy", description_server_privacy,
     additional_constraints_server_privacy, nullptr, 0, false, false},
    {"tcp_frame_size_tuning", description_tcp_frame_size_tuning,
     additional_constraints_tcp_frame_size_tuning, nullptr, 0, false, true},
    {"tcp_rcv_lowat", description_tcp_rcv_lowat,
     additional_constraints_tcp_rcv_lowat, nullptr, 0, false, true},
    {"trace_record_callops", description_trace_record_callops,
     additional_constraints_trace_record_callops, nullptr, 0, true, true},
    {"unconstrained_max_quota_buffer_size",
     description_unconstrained_max_quota_buffer_size,
     additional_constraints_unconstrained_max_quota_buffer_size, nullptr, 0,
     false, true},
    {"work_serializer_clears_time_cache",
     description_work_serializer_clears_time_cache,
     additional_constraints_work_serializer_clears_time_cache, nullptr, 0, true,
     true},
    {"work_serializer_dispatch", description_work_serializer_dispatch,
     additional_constraints_work_serializer_dispatch,
     required_experiments_work_serializer_dispatch, 1, true, true},
};

}  // namespace grpc_core
#endif
#endif
