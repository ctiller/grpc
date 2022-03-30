/*
 *
 * Copyright 2015 gRPC authors.
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

#include "src/core/lib/surface/channel.h"

#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <atomic>

#include <grpc/compression.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/string_util.h>

#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/channel/channel_stack_builder_impl.h"
#include "src/core/lib/channel/channel_trace.h"
#include "src/core/lib/channel/channelz.h"
#include "src/core/lib/channel/channelz_registry.h"
#include "src/core/lib/config/core_configuration.h"
#include "src/core/lib/debug/stats.h"
#include "src/core/lib/gpr/string.h"
#include "src/core/lib/gprpp/manual_constructor.h"
#include "src/core/lib/gprpp/memory.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/iomgr/iomgr.h"
#include "src/core/lib/resource_quota/api.h"
#include "src/core/lib/resource_quota/memory_quota.h"
#include "src/core/lib/slice/slice_internal.h"
#include "src/core/lib/surface/api_trace.h"
#include "src/core/lib/surface/call.h"
#include "src/core/lib/surface/channel.h"
#include "src/core/lib/surface/channel_stack_type.h"
#include "src/core/lib/transport/error_utils.h"

namespace grpc_core {

Channel::Channel(bool is_client, std::string target, ChannelArgs channel_args,
                 grpc_compression_options compression_options,
                 RefCountedPtr<grpc_channel_stack> channel_stack)
    : is_client_(is_client),
      compression_options_(compression_options),
      call_size_estimate_(channel_stack->call_stack_size +
                          grpc_call_get_initial_size_estimate()),
      channelz_node_(channel_args.GetObjectRef<channelz::ChannelNode>()),
      allocator_(channel_args.GetObject<ResourceQuota>()
                     ->memory_quota()
                     ->CreateMemoryOwner(target)),
      target_(std::move(target)),
      channel_stack_(std::move(channel_stack)) {}

}  // namespace grpc_core

grpc_channel* grpc_channel_create_with_builder(
    grpc_core::ChannelStackBuilder* builder,
    grpc_channel_stack_type channel_stack_type, grpc_error_handle* error) {
  std::string target(builder->target());
  auto channel_args = grpc_core::ChannelArgs::FromC(builder->channel_args());
  if (channel_stack_type == GRPC_SERVER_CHANNEL) {
    GRPC_STATS_INC_SERVER_CHANNELS_CREATED();
  } else {
    GRPC_STATS_INC_CLIENT_CHANNELS_CREATED();
  }
  absl::StatusOr<grpc_core::RefCountedPtr<grpc_channel_stack>> r =
      builder->Build();
  if (!r.ok()) {
    auto* builder_error = absl_status_to_grpc_error(r.status());
    gpr_log(GPR_ERROR, "channel stack builder failed: %s",
            grpc_error_std_string(builder_error).c_str());
    if (error != nullptr) {
      *error = builder_error;
    } else {
      GRPC_ERROR_UNREF(builder_error);
    }
    return nullptr;
  }

  grpc_compression_options compression_options;
  grpc_compression_options_init(&compression_options);
  auto default_level =
      channel_args.GetInt(GRPC_COMPRESSION_CHANNEL_DEFAULT_LEVEL);
  if (default_level.has_value()) {
    compression_options.default_level.is_set = true;
    compression_options.default_level.level = grpc_core::Clamp(
        static_cast<grpc_compression_level>(*default_level),
        GRPC_COMPRESS_LEVEL_NONE,
        static_cast<grpc_compression_level>(GRPC_COMPRESS_LEVEL_COUNT - 1));
  }
  auto default_algorithm =
      channel_args.GetInt(GRPC_COMPRESSION_CHANNEL_DEFAULT_ALGORITHM);
  if (default_algorithm.has_value()) {
    compression_options.default_algorithm.is_set = true;
    compression_options.default_algorithm.algorithm = grpc_core::Clamp(
        static_cast<grpc_compression_algorithm>(*default_algorithm),
        GRPC_COMPRESS_NONE,
        static_cast<grpc_compression_algorithm>(GRPC_COMPRESS_ALGORITHMS_COUNT -
                                                1));
  }
  compression_options.enabled_algorithms_bitset =
      channel_args.GetInt(GRPC_COMPRESSION_CHANNEL_ENABLED_ALGORITHMS_BITSET)
          .value_or(0) |
      1 /* always support no compression */;

  return (new grpc_core::Channel(
              grpc_channel_stack_type_is_client(channel_stack_type),
              std::string(builder->target()), std::move(channel_args),
              compression_options, std::move(*r)))
      ->c_ptr();
}

static grpc_core::UniquePtr<char> get_default_authority(
    const grpc_channel_args* input_args) {
  bool has_default_authority = false;
  char* ssl_override = nullptr;
  grpc_core::UniquePtr<char> default_authority;
  const size_t num_args = input_args != nullptr ? input_args->num_args : 0;
  for (size_t i = 0; i < num_args; ++i) {
    if (0 == strcmp(input_args->args[i].key, GRPC_ARG_DEFAULT_AUTHORITY)) {
      has_default_authority = true;
    } else if (0 == strcmp(input_args->args[i].key,
                           GRPC_SSL_TARGET_NAME_OVERRIDE_ARG)) {
      ssl_override = grpc_channel_arg_get_string(&input_args->args[i]);
    }
  }
  if (!has_default_authority && ssl_override != nullptr) {
    default_authority.reset(gpr_strdup(ssl_override));
  }
  return default_authority;
}

static grpc_channel_args* build_channel_args(
    const grpc_channel_args* input_args, char* default_authority) {
  grpc_arg new_args[1];
  size_t num_new_args = 0;
  if (default_authority != nullptr) {
    new_args[num_new_args++] = grpc_channel_arg_string_create(
        const_cast<char*>(GRPC_ARG_DEFAULT_AUTHORITY), default_authority);
  }
  return grpc_channel_args_copy_and_add(input_args, new_args, num_new_args);
}

namespace {

void* channelz_node_copy(void* p) {
  grpc_core::channelz::ChannelNode* node =
      static_cast<grpc_core::channelz::ChannelNode*>(p);
  node->Ref().release();
  return p;
}
void channelz_node_destroy(void* p) {
  grpc_core::channelz::ChannelNode* node =
      static_cast<grpc_core::channelz::ChannelNode*>(p);
  node->Unref();
}
int channelz_node_cmp(void* p1, void* p2) {
  return grpc_core::QsortCompare(p1, p2);
}
const grpc_arg_pointer_vtable channelz_node_arg_vtable = {
    channelz_node_copy, channelz_node_destroy, channelz_node_cmp};

void CreateChannelzNode(grpc_core::ChannelStackBuilder* builder) {
  const grpc_channel_args* args = builder->channel_args();
  // Check whether channelz is enabled.
  const bool channelz_enabled = grpc_channel_args_find_bool(
      args, GRPC_ARG_ENABLE_CHANNELZ, GRPC_ENABLE_CHANNELZ_DEFAULT);
  if (!channelz_enabled) return;
  // Get parameters needed to create the channelz node.
  const size_t channel_tracer_max_memory = grpc_channel_args_find_integer(
      args, GRPC_ARG_MAX_CHANNEL_TRACE_EVENT_MEMORY_PER_NODE,
      {GRPC_MAX_CHANNEL_TRACE_EVENT_MEMORY_PER_NODE_DEFAULT, 0, INT_MAX});
  const bool is_internal_channel = grpc_channel_args_find_bool(
      args, GRPC_ARG_CHANNELZ_IS_INTERNAL_CHANNEL, false);
  // Create the channelz node.
  std::string target(builder->target());
  grpc_core::RefCountedPtr<grpc_core::channelz::ChannelNode> channelz_node =
      grpc_core::MakeRefCounted<grpc_core::channelz::ChannelNode>(
          target.c_str(), channel_tracer_max_memory, is_internal_channel);
  channelz_node->AddTraceEvent(
      grpc_core::channelz::ChannelTrace::Severity::Info,
      grpc_slice_from_static_string("Channel created"));
  // Add channelz node to channel args.
  // We remove the is_internal_channel arg, since we no longer need it.
  grpc_arg new_arg = grpc_channel_arg_pointer_create(
      const_cast<char*>(GRPC_ARG_CHANNELZ_CHANNEL_NODE), channelz_node.get(),
      &channelz_node_arg_vtable);
  const char* args_to_remove[] = {GRPC_ARG_CHANNELZ_IS_INTERNAL_CHANNEL};
  grpc_channel_args* new_args = grpc_channel_args_copy_and_add_and_remove(
      args, args_to_remove, GPR_ARRAY_SIZE(args_to_remove), &new_arg, 1);
  builder->SetChannelArgs(new_args);
  grpc_channel_args_destroy(new_args);
}

}  // namespace

grpc_channel* grpc_channel_create_internal(
    const char* target, const grpc_channel_args* input_args,
    grpc_channel_stack_type channel_stack_type,
    grpc_transport* optional_transport, grpc_error_handle* error) {
  // We need to make sure that grpc_shutdown() does not shut things down
  // until after the channel is destroyed.  However, the channel may not
  // actually be destroyed by the time grpc_channel_destroy() returns,
  // since there may be other existing refs to the channel.  If those
  // refs are held by things that are visible to the wrapped language
  // (such as outstanding calls on the channel), then the wrapped
  // language can be responsible for making sure that grpc_shutdown()
  // does not run until after those refs are released.  However, the
  // channel may also have refs to itself held internally for various
  // things that need to be cleaned up at channel destruction (e.g.,
  // LB policies, subchannels, etc), and because these refs are not
  // visible to the wrapped language, it cannot be responsible for
  // deferring grpc_shutdown() until after they are released.  To
  // accommodate that, we call grpc_init() here and then call
  // grpc_shutdown() when the channel is actually destroyed, thus
  // ensuring that shutdown is deferred until that point.
  grpc_init();
  grpc_core::ChannelStackBuilderImpl builder(
      grpc_channel_stack_type_string(channel_stack_type), channel_stack_type);
  const grpc_core::UniquePtr<char> default_authority =
      get_default_authority(input_args);
  grpc_channel_args* args =
      build_channel_args(input_args, default_authority.get());
  if (grpc_channel_stack_type_is_client(channel_stack_type)) {
    auto channel_args_mutator =
        grpc_channel_args_get_client_channel_creation_mutator();
    if (channel_args_mutator != nullptr) {
      args = channel_args_mutator(target, args, channel_stack_type);
    }
  }
  builder.SetChannelArgs(args).SetTarget(target).SetTransport(
      optional_transport);
  grpc_channel_args_destroy(args);
  if (!grpc_core::CoreConfiguration::Get().channel_init().CreateStack(
          &builder)) {
    grpc_shutdown();  // Since we won't call destroy_channel().
    return nullptr;
  }
  // We only need to do this for clients here. For servers, this will be
  // done in src/core/lib/surface/server.cc.
  if (grpc_channel_stack_type_is_client(channel_stack_type)) {
    CreateChannelzNode(&builder);
  }
  grpc_channel* channel =
      grpc_channel_create_with_builder(&builder, channel_stack_type, error);
  if (channel == nullptr) {
    grpc_shutdown();  // Since we won't call destroy_channel().
  }
  return channel;
}

namespace grpc_core {

void Channel::UpdateCallSizeEstimate(size_t size) {
  size_t cur = call_size_estimate_.load(std::memory_order_relaxed);
  if (cur < size) {
    // size grew: update estimate
    call_size_estimate_.compare_exchange_weak(
        cur, size, std::memory_order_relaxed, std::memory_order_relaxed);
    // if we lose: never mind, something else will likely update soon enough
  } else if (cur == size) {
    // no change: holding pattern
  } else if (cur > 0) {
    // size shrank: decrease estimate
    call_size_estimate_.compare_exchange_weak(
        cur, std::min(cur - 1, (255 * cur + size) / 256),
        std::memory_order_relaxed, std::memory_order_relaxed);
    // if we lose: never mind, something else will likely update soon enough
  }
}

}  // namespace grpc_core

char* grpc_channel_get_target(grpc_channel* channel) {
  GRPC_API_TRACE("grpc_channel_get_target(channel=%p)", 1, (channel));
  auto target = grpc_core::Channel::FromC(channel)->target();
  char* buffer = static_cast<char*>(gpr_zalloc(target.size() + 1));
  memcpy(buffer, target.data(), target.size());
  return buffer;
}

void grpc_channel_get_info(grpc_channel* channel,
                           const grpc_channel_info* channel_info) {
  grpc_core::ApplicationCallbackExecCtx callback_exec_ctx;
  grpc_core::ExecCtx exec_ctx;
  grpc_channel_element* elem = grpc_channel_stack_element(
      grpc_core::Channel::FromC(channel)->channel_stack(), 0);
  elem->filter->get_channel_info(elem, channel_info);
}

void grpc_channel_reset_connect_backoff(grpc_channel* channel) {
  grpc_core::ApplicationCallbackExecCtx callback_exec_ctx;
  grpc_core::ExecCtx exec_ctx;
  GRPC_API_TRACE("grpc_channel_reset_connect_backoff(channel=%p)", 1,
                 (channel));
  grpc_transport_op* op = grpc_make_transport_op(nullptr);
  op->reset_connect_backoff = true;
  grpc_channel_element* elem = grpc_channel_stack_element(
      grpc_core::Channel::FromC(channel)->channel_stack(), 0);
  elem->filter->start_transport_op(elem, op);
}

static grpc_call* grpc_channel_create_call_internal(
    grpc_channel* c_channel, grpc_call* parent_call, uint32_t propagation_mask,
    grpc_completion_queue* cq, grpc_pollset_set* pollset_set_alternative,
    grpc_core::Slice path, absl::optional<grpc_core::Slice> authority,
    grpc_core::Timestamp deadline) {
  auto* channel = grpc_core::Channel::FromC(c_channel);
  GPR_ASSERT(channel->is_client());
  GPR_ASSERT(!(cq != nullptr && pollset_set_alternative != nullptr));

  grpc_call_create_args args;
  args.channel = channel;
  args.server = nullptr;
  args.parent = parent_call;
  args.propagation_mask = propagation_mask;
  args.cq = cq;
  args.pollset_set_alternative = pollset_set_alternative;
  args.server_transport_data = nullptr;
  args.path = std::move(path);
  args.authority = std::move(authority);
  args.send_deadline = deadline;

  grpc_call* call;
  GRPC_LOG_IF_ERROR("call_create", grpc_call_create(&args, &call));
  return call;
}

grpc_call* grpc_channel_create_call(grpc_channel* channel,
                                    grpc_call* parent_call,
                                    uint32_t propagation_mask,
                                    grpc_completion_queue* completion_queue,
                                    grpc_slice method, const grpc_slice* host,
                                    gpr_timespec deadline, void* reserved) {
  GPR_ASSERT(!reserved);
  grpc_core::ApplicationCallbackExecCtx callback_exec_ctx;
  grpc_core::ExecCtx exec_ctx;
  grpc_call* call = grpc_channel_create_call_internal(
      channel, parent_call, propagation_mask, completion_queue, nullptr,
      grpc_core::Slice(grpc_slice_ref_internal(method)),
      host != nullptr
          ? absl::optional<grpc_core::Slice>(grpc_slice_ref_internal(*host))
          : absl::nullopt,
      grpc_core::Timestamp::FromTimespecRoundUp(deadline));

  return call;
}

grpc_call* grpc_channel_create_pollset_set_call(
    grpc_channel* channel, grpc_call* parent_call, uint32_t propagation_mask,
    grpc_pollset_set* pollset_set, const grpc_slice& method,
    const grpc_slice* host, grpc_core::Timestamp deadline, void* reserved) {
  GPR_ASSERT(!reserved);
  return grpc_channel_create_call_internal(
      channel, parent_call, propagation_mask, nullptr, pollset_set,
      grpc_core::Slice(method),
      host != nullptr
          ? absl::optional<grpc_core::Slice>(grpc_slice_ref_internal(*host))
          : absl::nullopt,
      deadline);
}

namespace grpc_core {

RegisteredCall::RegisteredCall(const char* method_arg, const char* host_arg) {
  path = Slice::FromCopiedString(method_arg);
  if (host_arg != nullptr && host_arg[0] != 0) {
    authority = Slice::FromCopiedString(host_arg);
  }
}

RegisteredCall::RegisteredCall(const RegisteredCall& other)
    : path(other.path.Ref()) {
  if (other.authority.has_value()) {
    authority = other.authority->Ref();
  }
}

RegisteredCall::~RegisteredCall() {}

}  // namespace grpc_core

void* grpc_channel_register_call(grpc_channel* channel, const char* method,
                                 const char* host, void* reserved) {
  GRPC_API_TRACE(
      "grpc_channel_register_call(channel=%p, method=%s, host=%s, reserved=%p)",
      4, (channel, method, host, reserved));
  GPR_ASSERT(!reserved);
  grpc_core::ApplicationCallbackExecCtx callback_exec_ctx;
  grpc_core::ExecCtx exec_ctx;
  return grpc_core::Channel::FromC(channel)->RegisterCall(method, host);
}

namespace grpc_core {

RegisteredCall* Channel::RegisterCall(const char* method, const char* host) {
  MutexLock lock(&registration_table_.mu);
  registration_table_.method_registration_attempts++;
  auto key = std::make_pair(std::string(host != nullptr ? host : ""),
                            std::string(method != nullptr ? method : ""));
  auto rc_posn = registration_table_.map.find(key);
  if (rc_posn != registration_table_.map.end()) {
    return &rc_posn->second;
  }
  auto insertion_result = registration_table_.map.insert(
      {std::move(key), RegisteredCall(method, host)});
  return &insertion_result.first->second;
}

}  // namespace grpc_core

grpc_call* grpc_channel_create_registered_call(
    grpc_channel* channel, grpc_call* parent_call, uint32_t propagation_mask,
    grpc_completion_queue* completion_queue, void* registered_call_handle,
    gpr_timespec deadline, void* reserved) {
  grpc_core::RegisteredCall* rc =
      static_cast<grpc_core::RegisteredCall*>(registered_call_handle);
  GRPC_API_TRACE(
      "grpc_channel_create_registered_call("
      "channel=%p, parent_call=%p, propagation_mask=%x, completion_queue=%p, "
      "registered_call_handle=%p, "
      "deadline=gpr_timespec { tv_sec: %" PRId64
      ", tv_nsec: %d, clock_type: %d }, "
      "reserved=%p)",
      9,
      (channel, parent_call, (unsigned)propagation_mask, completion_queue,
       registered_call_handle, deadline.tv_sec, deadline.tv_nsec,
       (int)deadline.clock_type, reserved));
  GPR_ASSERT(!reserved);
  grpc_core::ApplicationCallbackExecCtx callback_exec_ctx;
  grpc_core::ExecCtx exec_ctx;
  grpc_call* call = grpc_channel_create_call_internal(
      channel, parent_call, propagation_mask, completion_queue, nullptr,
      rc->path.Ref(),
      rc->authority.has_value()
          ? absl::optional<grpc_core::Slice>(rc->authority->Ref())
          : absl::nullopt,
      grpc_core::Timestamp::FromTimespecRoundUp(deadline));

  return call;
}

namespace grpc_core {

Channel::~Channel() {
  if (channelz_node_ != nullptr) {
    channelz_node_->AddTraceEvent(
        channelz::ChannelTrace::Severity::Info,
        grpc_slice_from_static_string("Channel destroyed"));
  }
}

}  // namespace grpc_core

void grpc_channel_destroy_internal(grpc_channel* c_channel) {
  grpc_core::RefCountedPtr<grpc_core::Channel> channel(
      grpc_core::Channel::FromC(c_channel));
  grpc_transport_op* op = grpc_make_transport_op(nullptr);
  grpc_channel_element* elem;
  GRPC_API_TRACE("grpc_channel_destroy(channel=%p)", 1, (c_channel));
  op->disconnect_with_error =
      GRPC_ERROR_CREATE_FROM_STATIC_STRING("Channel Destroyed");
  elem = grpc_channel_stack_element(channel->channel_stack(), 0);
  elem->filter->start_transport_op(elem, op);
}

void grpc_channel_destroy(grpc_channel* channel) {
  grpc_core::ApplicationCallbackExecCtx callback_exec_ctx;
  grpc_core::ExecCtx exec_ctx;
  grpc_channel_destroy_internal(channel);
}
