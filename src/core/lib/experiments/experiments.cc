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

#include <grpc/support/port_platform.h>

#include "src/core/lib/experiments/experiments.h"

#ifndef GRPC_EXPERIMENTS_ARE_FINAL

#if defined(GRPC_CFSTREAM)
namespace {
const char* const description_tcp_frame_size_tuning =
    "If set, enables TCP to use RPC size estimation made by higher layers. TCP "
    "would not indicate completion of a read operation until a specified "
    "number of bytes have been read over the socket. Buffers are also "
    "allocated according to estimated RPC sizes.";
const char* const additional_constraints_tcp_frame_size_tuning = "{}";
const char* const description_tcp_rcv_lowat =
    "Use SO_RCVLOWAT to avoid wakeups on the read path.";
const char* const additional_constraints_tcp_rcv_lowat = "{}";
const char* const description_peer_state_based_framing =
    "If set, the max sizes of frames sent to lower layers is controlled based "
    "on the peer's memory pressure which is reflected in its max http2 frame "
    "size.";
const char* const additional_constraints_peer_state_based_framing = "{}";
const char* const description_memory_pressure_controller =
    "New memory pressure controller";
const char* const additional_constraints_memory_pressure_controller = "{}";
const char* const description_unconstrained_max_quota_buffer_size =
    "Discard the cap on the max free pool size for one memory allocator";
const char* const additional_constraints_unconstrained_max_quota_buffer_size =
    "{}";
const char* const description_event_engine_client =
    "Use EventEngine clients instead of iomgr's grpc_tcp_client";
const char* const additional_constraints_event_engine_client = "{}";
const char* const description_monitoring_experiment =
    "Placeholder experiment to prove/disprove our monitoring is working";
const char* const additional_constraints_monitoring_experiment = "{}";
const char* const description_promise_based_client_call =
    "If set, use the new gRPC promise based call code when it's appropriate "
    "(ie when all filters in a stack are promise based)";
const char* const additional_constraints_promise_based_client_call = "{}";
const char* const description_free_large_allocator =
    "If set, return all free bytes from a \042big\042 allocator";
const char* const additional_constraints_free_large_allocator = "{}";
const char* const description_promise_based_server_call =
    "If set, use the new gRPC promise based call code when it's appropriate "
    "(ie when all filters in a stack are promise based)";
const char* const additional_constraints_promise_based_server_call = "{}";
const char* const description_event_engine_listener =
    "Use EventEngine listeners instead of iomgr's grpc_tcp_server";
const char* const additional_constraints_event_engine_listener = "{}";
const char* const description_schedule_cancellation_over_write =
    "Allow cancellation op to be scheduled over a write";
const char* const additional_constraints_schedule_cancellation_over_write =
    "{}";
const char* const description_trace_record_callops =
    "Enables tracing of call batch initiation and completion.";
const char* const additional_constraints_trace_record_callops = "{}";
const char* const description_event_engine_dns =
    "If set, use EventEngine DNSResolver for client channel resolution";
const char* const additional_constraints_event_engine_dns = "{}";
const char* const description_client_privacy = "If set, client privacy";
const char* const additional_constraints_client_privacy = "{}";
const char* const description_canary_client_privacy =
    "If set, canary client privacy";
const char* const additional_constraints_canary_client_privacy = "{}";
const char* const description_server_privacy = "If set, server privacy";
const char* const additional_constraints_server_privacy = "{}";
const char* const description_keepalive_fix =
    "Allows overriding keepalive_permit_without_calls. Refer "
    "https://github.com/grpc/grpc/pull/33428 for more information.";
const char* const additional_constraints_keepalive_fix = "{}";
const char* const description_keepalive_server_fix =
    "Allows overriding keepalive_permit_without_calls for servers. Refer "
    "https://github.com/grpc/grpc/pull/33917 for more information.";
const char* const additional_constraints_keepalive_server_fix = "{}";
const char* const description_work_serializer_dispatch =
    "Have the work serializer dispatch work to event engine for every "
    "callback, instead of running things inline in the first thread that "
    "successfully enqueues work.";
const char* const additional_constraints_work_serializer_dispatch = "{}";
const char* const description_lazier_stream_updates =
    "Allow streams to consume up to 50% of the incoming window before we force "
    "send a flow control update.";
const char* const additional_constraints_lazier_stream_updates = "{}";
const char* const description_jitter_max_idle =
    "Enable jitter on connection max idle times. Historically this jitter was "
    "only on max connection age, but it seems like this could smooth out some "
    "herding problems.";
const char* const additional_constraints_jitter_max_idle = "{}";
const char* const description_round_robin_delegate_to_pick_first =
    "Change round_robin code to delegate to pick_first as per dualstack "
    "backend design.";
const char* const additional_constraints_round_robin_delegate_to_pick_first =
    "{}";
const char* const description_wrr_delegate_to_pick_first =
    "Change WRR code to delegate to pick_first as per dualstack backend "
    "design.";
const char* const additional_constraints_wrr_delegate_to_pick_first = "{}";
const char* const description_pick_first_happy_eyeballs =
    "Use Happy Eyeballs in pick_first.";
const char* const additional_constraints_pick_first_happy_eyeballs = "{}";
const char* const description_combiner_offload_to_event_engine =
    "Offload Combiner work onto the EventEngine instead of the Executor.";
const char* const additional_constraints_combiner_offload_to_event_engine =
    "{}";
const char* const description_registered_method_lookup_in_transport =
    "Change registered method's lookup point to transport";
const char* const additional_constraints_registered_method_lookup_in_transport =
    "{}";
const char* const description_call_status_override_on_cancellation =
    "Avoid overriding call status of successfully finished calls if it races "
    "with cancellation.";
const char* const additional_constraints_call_status_override_on_cancellation =
    "{}";
const char* const description_block_excessive_requests_before_settings_ack =
    "If set, block excessive requests before receiving SETTINGS ACK.";
const char* const
    additional_constraints_block_excessive_requests_before_settings_ack = "{}";
#ifdef NDEBUG
const bool kDefaultForDebugOnly = false;
#else
const bool kDefaultForDebugOnly = true;
#endif
}  // namespace

namespace grpc_core {

const ExperimentMetadata g_experiment_metadata[] = {
    {"tcp_frame_size_tuning", description_tcp_frame_size_tuning,
     additional_constraints_tcp_frame_size_tuning, false, true},
    {"tcp_rcv_lowat", description_tcp_rcv_lowat,
     additional_constraints_tcp_rcv_lowat, false, true},
    {"peer_state_based_framing", description_peer_state_based_framing,
     additional_constraints_peer_state_based_framing, false, true},
    {"memory_pressure_controller", description_memory_pressure_controller,
     additional_constraints_memory_pressure_controller, false, true},
    {"unconstrained_max_quota_buffer_size",
     description_unconstrained_max_quota_buffer_size,
     additional_constraints_unconstrained_max_quota_buffer_size, false, true},
    {"event_engine_client", description_event_engine_client,
     additional_constraints_event_engine_client, false, true},
    {"monitoring_experiment", description_monitoring_experiment,
     additional_constraints_monitoring_experiment, true, true},
    {"promise_based_client_call", description_promise_based_client_call,
     additional_constraints_promise_based_client_call, false, true},
    {"free_large_allocator", description_free_large_allocator,
     additional_constraints_free_large_allocator, false, true},
    {"promise_based_server_call", description_promise_based_server_call,
     additional_constraints_promise_based_server_call, false, true},
    {"event_engine_listener", description_event_engine_listener,
     additional_constraints_event_engine_listener, false, true},
    {"schedule_cancellation_over_write",
     description_schedule_cancellation_over_write,
     additional_constraints_schedule_cancellation_over_write, false, true},
    {"trace_record_callops", description_trace_record_callops,
     additional_constraints_trace_record_callops, false, true},
    {"event_engine_dns", description_event_engine_dns,
     additional_constraints_event_engine_dns, false, false},
    {"client_privacy", description_client_privacy,
     additional_constraints_client_privacy, false, false},
    {"canary_client_privacy", description_canary_client_privacy,
     additional_constraints_canary_client_privacy, false, false},
    {"server_privacy", description_server_privacy,
     additional_constraints_server_privacy, false, false},
    {"keepalive_fix", description_keepalive_fix,
     additional_constraints_keepalive_fix, false, false},
    {"keepalive_server_fix", description_keepalive_server_fix,
     additional_constraints_keepalive_server_fix, false, false},
    {"work_serializer_dispatch", description_work_serializer_dispatch,
     additional_constraints_work_serializer_dispatch, false, true},
    {"lazier_stream_updates", description_lazier_stream_updates,
     additional_constraints_lazier_stream_updates, true, true},
    {"jitter_max_idle", description_jitter_max_idle,
     additional_constraints_jitter_max_idle, true, true},
    {"round_robin_delegate_to_pick_first",
     description_round_robin_delegate_to_pick_first,
     additional_constraints_round_robin_delegate_to_pick_first, true, true},
    {"wrr_delegate_to_pick_first", description_wrr_delegate_to_pick_first,
     additional_constraints_wrr_delegate_to_pick_first, true, true},
    {"pick_first_happy_eyeballs", description_pick_first_happy_eyeballs,
     additional_constraints_pick_first_happy_eyeballs, true, true},
    {"combiner_offload_to_event_engine",
     description_combiner_offload_to_event_engine,
     additional_constraints_combiner_offload_to_event_engine, true, true},
    {"registered_method_lookup_in_transport",
     description_registered_method_lookup_in_transport,
     additional_constraints_registered_method_lookup_in_transport, true, true},
    {"call_status_override_on_cancellation",
     description_call_status_override_on_cancellation,
     additional_constraints_call_status_override_on_cancellation,
     kDefaultForDebugOnly, true},
    {"block_excessive_requests_before_settings_ack",
     description_block_excessive_requests_before_settings_ack,
     additional_constraints_block_excessive_requests_before_settings_ack, false,
     true},
};

}  // namespace grpc_core

#elif defined(GPR_WINDOWS)
namespace {
const char* const description_tcp_frame_size_tuning =
    "If set, enables TCP to use RPC size estimation made by higher layers. TCP "
    "would not indicate completion of a read operation until a specified "
    "number of bytes have been read over the socket. Buffers are also "
    "allocated according to estimated RPC sizes.";
const char* const additional_constraints_tcp_frame_size_tuning = "{}";
const char* const description_tcp_rcv_lowat =
    "Use SO_RCVLOWAT to avoid wakeups on the read path.";
const char* const additional_constraints_tcp_rcv_lowat = "{}";
const char* const description_peer_state_based_framing =
    "If set, the max sizes of frames sent to lower layers is controlled based "
    "on the peer's memory pressure which is reflected in its max http2 frame "
    "size.";
const char* const additional_constraints_peer_state_based_framing = "{}";
const char* const description_memory_pressure_controller =
    "New memory pressure controller";
const char* const additional_constraints_memory_pressure_controller = "{}";
const char* const description_unconstrained_max_quota_buffer_size =
    "Discard the cap on the max free pool size for one memory allocator";
const char* const additional_constraints_unconstrained_max_quota_buffer_size =
    "{}";
const char* const description_event_engine_client =
    "Use EventEngine clients instead of iomgr's grpc_tcp_client";
const char* const additional_constraints_event_engine_client = "{}";
const char* const description_monitoring_experiment =
    "Placeholder experiment to prove/disprove our monitoring is working";
const char* const additional_constraints_monitoring_experiment = "{}";
const char* const description_promise_based_client_call =
    "If set, use the new gRPC promise based call code when it's appropriate "
    "(ie when all filters in a stack are promise based)";
const char* const additional_constraints_promise_based_client_call = "{}";
const char* const description_free_large_allocator =
    "If set, return all free bytes from a \042big\042 allocator";
const char* const additional_constraints_free_large_allocator = "{}";
const char* const description_promise_based_server_call =
    "If set, use the new gRPC promise based call code when it's appropriate "
    "(ie when all filters in a stack are promise based)";
const char* const additional_constraints_promise_based_server_call = "{}";
const char* const description_event_engine_listener =
    "Use EventEngine listeners instead of iomgr's grpc_tcp_server";
const char* const additional_constraints_event_engine_listener = "{}";
const char* const description_schedule_cancellation_over_write =
    "Allow cancellation op to be scheduled over a write";
const char* const additional_constraints_schedule_cancellation_over_write =
    "{}";
const char* const description_trace_record_callops =
    "Enables tracing of call batch initiation and completion.";
const char* const additional_constraints_trace_record_callops = "{}";
const char* const description_event_engine_dns =
    "If set, use EventEngine DNSResolver for client channel resolution";
const char* const additional_constraints_event_engine_dns = "{}";
const char* const description_client_privacy = "If set, client privacy";
const char* const additional_constraints_client_privacy = "{}";
const char* const description_canary_client_privacy =
    "If set, canary client privacy";
const char* const additional_constraints_canary_client_privacy = "{}";
const char* const description_server_privacy = "If set, server privacy";
const char* const additional_constraints_server_privacy = "{}";
const char* const description_keepalive_fix =
    "Allows overriding keepalive_permit_without_calls. Refer "
    "https://github.com/grpc/grpc/pull/33428 for more information.";
const char* const additional_constraints_keepalive_fix = "{}";
const char* const description_keepalive_server_fix =
    "Allows overriding keepalive_permit_without_calls for servers. Refer "
    "https://github.com/grpc/grpc/pull/33917 for more information.";
const char* const additional_constraints_keepalive_server_fix = "{}";
const char* const description_work_serializer_dispatch =
    "Have the work serializer dispatch work to event engine for every "
    "callback, instead of running things inline in the first thread that "
    "successfully enqueues work.";
const char* const additional_constraints_work_serializer_dispatch = "{}";
const char* const description_lazier_stream_updates =
    "Allow streams to consume up to 50% of the incoming window before we force "
    "send a flow control update.";
const char* const additional_constraints_lazier_stream_updates = "{}";
const char* const description_jitter_max_idle =
    "Enable jitter on connection max idle times. Historically this jitter was "
    "only on max connection age, but it seems like this could smooth out some "
    "herding problems.";
const char* const additional_constraints_jitter_max_idle = "{}";
const char* const description_round_robin_delegate_to_pick_first =
    "Change round_robin code to delegate to pick_first as per dualstack "
    "backend design.";
const char* const additional_constraints_round_robin_delegate_to_pick_first =
    "{}";
const char* const description_wrr_delegate_to_pick_first =
    "Change WRR code to delegate to pick_first as per dualstack backend "
    "design.";
const char* const additional_constraints_wrr_delegate_to_pick_first = "{}";
const char* const description_pick_first_happy_eyeballs =
    "Use Happy Eyeballs in pick_first.";
const char* const additional_constraints_pick_first_happy_eyeballs = "{}";
const char* const description_combiner_offload_to_event_engine =
    "Offload Combiner work onto the EventEngine instead of the Executor.";
const char* const additional_constraints_combiner_offload_to_event_engine =
    "{}";
const char* const description_registered_method_lookup_in_transport =
    "Change registered method's lookup point to transport";
const char* const additional_constraints_registered_method_lookup_in_transport =
    "{}";
const char* const description_call_status_override_on_cancellation =
    "Avoid overriding call status of successfully finished calls if it races "
    "with cancellation.";
const char* const additional_constraints_call_status_override_on_cancellation =
    "{}";
const char* const description_block_excessive_requests_before_settings_ack =
    "If set, block excessive requests before receiving SETTINGS ACK.";
const char* const
    additional_constraints_block_excessive_requests_before_settings_ack = "{}";
#ifdef NDEBUG
const bool kDefaultForDebugOnly = false;
#else
const bool kDefaultForDebugOnly = true;
#endif
}  // namespace

namespace grpc_core {

const ExperimentMetadata g_experiment_metadata[] = {
    {"tcp_frame_size_tuning", description_tcp_frame_size_tuning,
     additional_constraints_tcp_frame_size_tuning, false, true},
    {"tcp_rcv_lowat", description_tcp_rcv_lowat,
     additional_constraints_tcp_rcv_lowat, false, true},
    {"peer_state_based_framing", description_peer_state_based_framing,
     additional_constraints_peer_state_based_framing, false, true},
    {"memory_pressure_controller", description_memory_pressure_controller,
     additional_constraints_memory_pressure_controller, false, true},
    {"unconstrained_max_quota_buffer_size",
     description_unconstrained_max_quota_buffer_size,
     additional_constraints_unconstrained_max_quota_buffer_size, false, true},
    {"event_engine_client", description_event_engine_client,
     additional_constraints_event_engine_client, false, true},
    {"monitoring_experiment", description_monitoring_experiment,
     additional_constraints_monitoring_experiment, true, true},
    {"promise_based_client_call", description_promise_based_client_call,
     additional_constraints_promise_based_client_call, false, true},
    {"free_large_allocator", description_free_large_allocator,
     additional_constraints_free_large_allocator, false, true},
    {"promise_based_server_call", description_promise_based_server_call,
     additional_constraints_promise_based_server_call, false, true},
    {"event_engine_listener", description_event_engine_listener,
     additional_constraints_event_engine_listener, false, true},
    {"schedule_cancellation_over_write",
     description_schedule_cancellation_over_write,
     additional_constraints_schedule_cancellation_over_write, false, true},
    {"trace_record_callops", description_trace_record_callops,
     additional_constraints_trace_record_callops, false, true},
    {"event_engine_dns", description_event_engine_dns,
     additional_constraints_event_engine_dns, false, false},
    {"client_privacy", description_client_privacy,
     additional_constraints_client_privacy, false, false},
    {"canary_client_privacy", description_canary_client_privacy,
     additional_constraints_canary_client_privacy, false, false},
    {"server_privacy", description_server_privacy,
     additional_constraints_server_privacy, false, false},
    {"keepalive_fix", description_keepalive_fix,
     additional_constraints_keepalive_fix, false, false},
    {"keepalive_server_fix", description_keepalive_server_fix,
     additional_constraints_keepalive_server_fix, false, false},
    {"work_serializer_dispatch", description_work_serializer_dispatch,
     additional_constraints_work_serializer_dispatch, false, true},
    {"lazier_stream_updates", description_lazier_stream_updates,
     additional_constraints_lazier_stream_updates, true, true},
    {"jitter_max_idle", description_jitter_max_idle,
     additional_constraints_jitter_max_idle, true, true},
    {"round_robin_delegate_to_pick_first",
     description_round_robin_delegate_to_pick_first,
     additional_constraints_round_robin_delegate_to_pick_first, true, true},
    {"wrr_delegate_to_pick_first", description_wrr_delegate_to_pick_first,
     additional_constraints_wrr_delegate_to_pick_first, true, true},
    {"pick_first_happy_eyeballs", description_pick_first_happy_eyeballs,
     additional_constraints_pick_first_happy_eyeballs, true, true},
    {"combiner_offload_to_event_engine",
     description_combiner_offload_to_event_engine,
     additional_constraints_combiner_offload_to_event_engine, true, true},
    {"registered_method_lookup_in_transport",
     description_registered_method_lookup_in_transport,
     additional_constraints_registered_method_lookup_in_transport, true, true},
    {"call_status_override_on_cancellation",
     description_call_status_override_on_cancellation,
     additional_constraints_call_status_override_on_cancellation,
     kDefaultForDebugOnly, true},
    {"block_excessive_requests_before_settings_ack",
     description_block_excessive_requests_before_settings_ack,
     additional_constraints_block_excessive_requests_before_settings_ack, false,
     true},
};

}  // namespace grpc_core

#else
namespace {
const char* const description_tcp_frame_size_tuning =
    "If set, enables TCP to use RPC size estimation made by higher layers. TCP "
    "would not indicate completion of a read operation until a specified "
    "number of bytes have been read over the socket. Buffers are also "
    "allocated according to estimated RPC sizes.";
const char* const additional_constraints_tcp_frame_size_tuning = "{}";
const char* const description_tcp_rcv_lowat =
    "Use SO_RCVLOWAT to avoid wakeups on the read path.";
const char* const additional_constraints_tcp_rcv_lowat = "{}";
const char* const description_peer_state_based_framing =
    "If set, the max sizes of frames sent to lower layers is controlled based "
    "on the peer's memory pressure which is reflected in its max http2 frame "
    "size.";
const char* const additional_constraints_peer_state_based_framing = "{}";
const char* const description_memory_pressure_controller =
    "New memory pressure controller";
const char* const additional_constraints_memory_pressure_controller = "{}";
const char* const description_unconstrained_max_quota_buffer_size =
    "Discard the cap on the max free pool size for one memory allocator";
const char* const additional_constraints_unconstrained_max_quota_buffer_size =
    "{}";
const char* const description_event_engine_client =
    "Use EventEngine clients instead of iomgr's grpc_tcp_client";
const char* const additional_constraints_event_engine_client = "{}";
const char* const description_monitoring_experiment =
    "Placeholder experiment to prove/disprove our monitoring is working";
const char* const additional_constraints_monitoring_experiment = "{}";
const char* const description_promise_based_client_call =
    "If set, use the new gRPC promise based call code when it's appropriate "
    "(ie when all filters in a stack are promise based)";
const char* const additional_constraints_promise_based_client_call = "{}";
const char* const description_free_large_allocator =
    "If set, return all free bytes from a \042big\042 allocator";
const char* const additional_constraints_free_large_allocator = "{}";
const char* const description_promise_based_server_call =
    "If set, use the new gRPC promise based call code when it's appropriate "
    "(ie when all filters in a stack are promise based)";
const char* const additional_constraints_promise_based_server_call = "{}";
const char* const description_event_engine_listener =
    "Use EventEngine listeners instead of iomgr's grpc_tcp_server";
const char* const additional_constraints_event_engine_listener = "{}";
const char* const description_schedule_cancellation_over_write =
    "Allow cancellation op to be scheduled over a write";
const char* const additional_constraints_schedule_cancellation_over_write =
    "{}";
const char* const description_trace_record_callops =
    "Enables tracing of call batch initiation and completion.";
const char* const additional_constraints_trace_record_callops = "{}";
const char* const description_event_engine_dns =
    "If set, use EventEngine DNSResolver for client channel resolution";
const char* const additional_constraints_event_engine_dns = "{}";
const char* const description_client_privacy = "If set, client privacy";
const char* const additional_constraints_client_privacy = "{}";
const char* const description_canary_client_privacy =
    "If set, canary client privacy";
const char* const additional_constraints_canary_client_privacy = "{}";
const char* const description_server_privacy = "If set, server privacy";
const char* const additional_constraints_server_privacy = "{}";
const char* const description_keepalive_fix =
    "Allows overriding keepalive_permit_without_calls. Refer "
    "https://github.com/grpc/grpc/pull/33428 for more information.";
const char* const additional_constraints_keepalive_fix = "{}";
const char* const description_keepalive_server_fix =
    "Allows overriding keepalive_permit_without_calls for servers. Refer "
    "https://github.com/grpc/grpc/pull/33917 for more information.";
const char* const additional_constraints_keepalive_server_fix = "{}";
const char* const description_work_serializer_dispatch =
    "Have the work serializer dispatch work to event engine for every "
    "callback, instead of running things inline in the first thread that "
    "successfully enqueues work.";
const char* const additional_constraints_work_serializer_dispatch = "{}";
const char* const description_lazier_stream_updates =
    "Allow streams to consume up to 50% of the incoming window before we force "
    "send a flow control update.";
const char* const additional_constraints_lazier_stream_updates = "{}";
const char* const description_jitter_max_idle =
    "Enable jitter on connection max idle times. Historically this jitter was "
    "only on max connection age, but it seems like this could smooth out some "
    "herding problems.";
const char* const additional_constraints_jitter_max_idle = "{}";
const char* const description_round_robin_delegate_to_pick_first =
    "Change round_robin code to delegate to pick_first as per dualstack "
    "backend design.";
const char* const additional_constraints_round_robin_delegate_to_pick_first =
    "{}";
const char* const description_wrr_delegate_to_pick_first =
    "Change WRR code to delegate to pick_first as per dualstack backend "
    "design.";
const char* const additional_constraints_wrr_delegate_to_pick_first = "{}";
const char* const description_pick_first_happy_eyeballs =
    "Use Happy Eyeballs in pick_first.";
const char* const additional_constraints_pick_first_happy_eyeballs = "{}";
const char* const description_combiner_offload_to_event_engine =
    "Offload Combiner work onto the EventEngine instead of the Executor.";
const char* const additional_constraints_combiner_offload_to_event_engine =
    "{}";
const char* const description_registered_method_lookup_in_transport =
    "Change registered method's lookup point to transport";
const char* const additional_constraints_registered_method_lookup_in_transport =
    "{}";
const char* const description_call_status_override_on_cancellation =
    "Avoid overriding call status of successfully finished calls if it races "
    "with cancellation.";
const char* const additional_constraints_call_status_override_on_cancellation =
    "{}";
const char* const description_block_excessive_requests_before_settings_ack =
    "If set, block excessive requests before receiving SETTINGS ACK.";
const char* const
    additional_constraints_block_excessive_requests_before_settings_ack = "{}";
#ifdef NDEBUG
const bool kDefaultForDebugOnly = false;
#else
const bool kDefaultForDebugOnly = true;
#endif
}  // namespace

namespace grpc_core {

const ExperimentMetadata g_experiment_metadata[] = {
    {"tcp_frame_size_tuning", description_tcp_frame_size_tuning,
     additional_constraints_tcp_frame_size_tuning, false, true},
    {"tcp_rcv_lowat", description_tcp_rcv_lowat,
     additional_constraints_tcp_rcv_lowat, false, true},
    {"peer_state_based_framing", description_peer_state_based_framing,
     additional_constraints_peer_state_based_framing, false, true},
    {"memory_pressure_controller", description_memory_pressure_controller,
     additional_constraints_memory_pressure_controller, false, true},
    {"unconstrained_max_quota_buffer_size",
     description_unconstrained_max_quota_buffer_size,
     additional_constraints_unconstrained_max_quota_buffer_size, false, true},
    {"event_engine_client", description_event_engine_client,
     additional_constraints_event_engine_client, false, true},
    {"monitoring_experiment", description_monitoring_experiment,
     additional_constraints_monitoring_experiment, true, true},
    {"promise_based_client_call", description_promise_based_client_call,
     additional_constraints_promise_based_client_call, false, true},
    {"free_large_allocator", description_free_large_allocator,
     additional_constraints_free_large_allocator, false, true},
    {"promise_based_server_call", description_promise_based_server_call,
     additional_constraints_promise_based_server_call, false, true},
    {"event_engine_listener", description_event_engine_listener,
     additional_constraints_event_engine_listener, false, true},
    {"schedule_cancellation_over_write",
     description_schedule_cancellation_over_write,
     additional_constraints_schedule_cancellation_over_write, false, true},
    {"trace_record_callops", description_trace_record_callops,
     additional_constraints_trace_record_callops, false, true},
    {"event_engine_dns", description_event_engine_dns,
     additional_constraints_event_engine_dns, false, false},
    {"client_privacy", description_client_privacy,
     additional_constraints_client_privacy, false, false},
    {"canary_client_privacy", description_canary_client_privacy,
     additional_constraints_canary_client_privacy, false, false},
    {"server_privacy", description_server_privacy,
     additional_constraints_server_privacy, false, false},
    {"keepalive_fix", description_keepalive_fix,
     additional_constraints_keepalive_fix, false, false},
    {"keepalive_server_fix", description_keepalive_server_fix,
     additional_constraints_keepalive_server_fix, false, false},
    {"work_serializer_dispatch", description_work_serializer_dispatch,
     additional_constraints_work_serializer_dispatch, false, true},
    {"lazier_stream_updates", description_lazier_stream_updates,
     additional_constraints_lazier_stream_updates, true, true},
    {"jitter_max_idle", description_jitter_max_idle,
     additional_constraints_jitter_max_idle, true, true},
    {"round_robin_delegate_to_pick_first",
     description_round_robin_delegate_to_pick_first,
     additional_constraints_round_robin_delegate_to_pick_first, true, true},
    {"wrr_delegate_to_pick_first", description_wrr_delegate_to_pick_first,
     additional_constraints_wrr_delegate_to_pick_first, true, true},
    {"pick_first_happy_eyeballs", description_pick_first_happy_eyeballs,
     additional_constraints_pick_first_happy_eyeballs, true, true},
    {"combiner_offload_to_event_engine",
     description_combiner_offload_to_event_engine,
     additional_constraints_combiner_offload_to_event_engine, true, true},
    {"registered_method_lookup_in_transport",
     description_registered_method_lookup_in_transport,
     additional_constraints_registered_method_lookup_in_transport, true, true},
    {"call_status_override_on_cancellation",
     description_call_status_override_on_cancellation,
     additional_constraints_call_status_override_on_cancellation,
     kDefaultForDebugOnly, true},
    {"block_excessive_requests_before_settings_ack",
     description_block_excessive_requests_before_settings_ack,
     additional_constraints_block_excessive_requests_before_settings_ack, false,
     true},
};

}  // namespace grpc_core
#endif
#endif
