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

#include "src/core/lib/channel/connected_channel.h"

#include <inttypes.h>
#include <string.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/base/thread_annotations.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"

#include <grpc/impl/codegen/grpc_types.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>

#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/channel/channel_fwd.h"
#include "src/core/lib/channel/channel_stack.h"
#include "src/core/lib/channel/context.h"
#include "src/core/lib/debug/trace.h"
#include "src/core/lib/gpr/alloc.h"
#include "src/core/lib/gprpp/debug_location.h"
#include "src/core/lib/gprpp/match.h"
#include "src/core/lib/gprpp/orphanable.h"
#include "src/core/lib/gprpp/status_helper.h"
#include "src/core/lib/gprpp/sync.h"
#include "src/core/lib/iomgr/call_combiner.h"
#include "src/core/lib/iomgr/closure.h"
#include "src/core/lib/iomgr/error.h"
#include "src/core/lib/iomgr/exec_ctx.h"
#include "src/core/lib/iomgr/polling_entity.h"
#include "src/core/lib/promise/activity.h"
#include "src/core/lib/promise/arena_promise.h"
#include "src/core/lib/promise/context.h"
#include "src/core/lib/promise/latch.h"
#include "src/core/lib/promise/pipe.h"
#include "src/core/lib/promise/poll.h"
#include "src/core/lib/resource_quota/arena.h"
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/surface/call.h"
#include "src/core/lib/surface/call_trace.h"
#include "src/core/lib/surface/channel_stack_type.h"
#include "src/core/lib/transport/metadata_batch.h"
#include "src/core/lib/transport/transport.h"
#include "src/core/lib/transport/transport_fwd.h"
#include "src/core/lib/transport/transport_impl.h"

#define MAX_BUFFER_LENGTH 8192

typedef struct connected_channel_channel_data {
  grpc_transport* transport;
} channel_data;

struct callback_state {
  grpc_closure closure;
  grpc_closure* original_closure;
  grpc_core::CallCombiner* call_combiner;
  const char* reason;
};
typedef struct connected_channel_call_data {
  grpc_core::CallCombiner* call_combiner;
  // Closures used for returning results on the call combiner.
  callback_state on_complete[6];  // Max number of pending batches.
  callback_state recv_initial_metadata_ready;
  callback_state recv_message_ready;
  callback_state recv_trailing_metadata_ready;
} call_data;

static void run_in_call_combiner(void* arg, grpc_error_handle error) {
  callback_state* state = static_cast<callback_state*>(arg);
  GRPC_CALL_COMBINER_START(state->call_combiner, state->original_closure, error,
                           state->reason);
}

static void run_cancel_in_call_combiner(void* arg, grpc_error_handle error) {
  run_in_call_combiner(arg, error);
  gpr_free(arg);
}

static void intercept_callback(call_data* calld, callback_state* state,
                               bool free_when_done, const char* reason,
                               grpc_closure** original_closure) {
  state->original_closure = *original_closure;
  state->call_combiner = calld->call_combiner;
  state->reason = reason;
  *original_closure = GRPC_CLOSURE_INIT(
      &state->closure,
      free_when_done ? run_cancel_in_call_combiner : run_in_call_combiner,
      state, grpc_schedule_on_exec_ctx);
}

static callback_state* get_state_for_batch(
    call_data* calld, grpc_transport_stream_op_batch* batch) {
  if (batch->send_initial_metadata) return &calld->on_complete[0];
  if (batch->send_message) return &calld->on_complete[1];
  if (batch->send_trailing_metadata) return &calld->on_complete[2];
  if (batch->recv_initial_metadata) return &calld->on_complete[3];
  if (batch->recv_message) return &calld->on_complete[4];
  if (batch->recv_trailing_metadata) return &calld->on_complete[5];
  GPR_UNREACHABLE_CODE(return nullptr);
}

/* We perform a small hack to locate transport data alongside the connected
   channel data in call allocations, to allow everything to be pulled in minimal
   cache line requests */
#define TRANSPORT_STREAM_FROM_CALL_DATA(calld) \
  ((grpc_stream*)(((char*)(calld)) +           \
                  GPR_ROUND_UP_TO_ALIGNMENT_SIZE(sizeof(call_data))))
#define CALL_DATA_FROM_TRANSPORT_STREAM(transport_stream) \
  ((call_data*)(((char*)(transport_stream)) -             \
                GPR_ROUND_UP_TO_ALIGNMENT_SIZE(sizeof(call_data))))

/* Intercept a call operation and either push it directly up or translate it
   into transport stream operations */
static void connected_channel_start_transport_stream_op_batch(
    grpc_call_element* elem, grpc_transport_stream_op_batch* batch) {
  call_data* calld = static_cast<call_data*>(elem->call_data);
  channel_data* chand = static_cast<channel_data*>(elem->channel_data);
  if (batch->recv_initial_metadata) {
    callback_state* state = &calld->recv_initial_metadata_ready;
    intercept_callback(
        calld, state, false, "recv_initial_metadata_ready",
        &batch->payload->recv_initial_metadata.recv_initial_metadata_ready);
  }
  if (batch->recv_message) {
    callback_state* state = &calld->recv_message_ready;
    intercept_callback(calld, state, false, "recv_message_ready",
                       &batch->payload->recv_message.recv_message_ready);
  }
  if (batch->recv_trailing_metadata) {
    callback_state* state = &calld->recv_trailing_metadata_ready;
    intercept_callback(
        calld, state, false, "recv_trailing_metadata_ready",
        &batch->payload->recv_trailing_metadata.recv_trailing_metadata_ready);
  }
  if (batch->cancel_stream) {
    // There can be more than one cancellation batch in flight at any
    // given time, so we can't just pick out a fixed index into
    // calld->on_complete like we can for the other ops.  However,
    // cancellation isn't in the fast path, so we just allocate a new
    // closure for each one.
    callback_state* state =
        static_cast<callback_state*>(gpr_malloc(sizeof(*state)));
    intercept_callback(calld, state, true, "on_complete (cancel_stream)",
                       &batch->on_complete);
  } else if (batch->on_complete != nullptr) {
    callback_state* state = get_state_for_batch(calld, batch);
    intercept_callback(calld, state, false, "on_complete", &batch->on_complete);
  }
  grpc_transport_perform_stream_op(
      chand->transport, TRANSPORT_STREAM_FROM_CALL_DATA(calld), batch);
  GRPC_CALL_COMBINER_STOP(calld->call_combiner, "passed batch to transport");
}

static void connected_channel_start_transport_op(grpc_channel_element* elem,
                                                 grpc_transport_op* op) {
  channel_data* chand = static_cast<channel_data*>(elem->channel_data);
  grpc_transport_perform_op(chand->transport, op);
}

/* Constructor for call_data */
static grpc_error_handle connected_channel_init_call_elem(
    grpc_call_element* elem, const grpc_call_element_args* args) {
  call_data* calld = static_cast<call_data*>(elem->call_data);
  channel_data* chand = static_cast<channel_data*>(elem->channel_data);
  calld->call_combiner = args->call_combiner;
  int r = grpc_transport_init_stream(
      chand->transport, TRANSPORT_STREAM_FROM_CALL_DATA(calld),
      &args->call_stack->refcount, args->server_transport_data, args->arena);
  return r == 0 ? absl::OkStatus()
                : GRPC_ERROR_CREATE("transport stream initialization failed");
}

static void set_pollset_or_pollset_set(grpc_call_element* elem,
                                       grpc_polling_entity* pollent) {
  call_data* calld = static_cast<call_data*>(elem->call_data);
  channel_data* chand = static_cast<channel_data*>(elem->channel_data);
  grpc_transport_set_pops(chand->transport,
                          TRANSPORT_STREAM_FROM_CALL_DATA(calld), pollent);
}

/* Destructor for call_data */
static void connected_channel_destroy_call_elem(
    grpc_call_element* elem, const grpc_call_final_info* /*final_info*/,
    grpc_closure* then_schedule_closure) {
  call_data* calld = static_cast<call_data*>(elem->call_data);
  channel_data* chand = static_cast<channel_data*>(elem->channel_data);
  grpc_transport_destroy_stream(chand->transport,
                                TRANSPORT_STREAM_FROM_CALL_DATA(calld),
                                then_schedule_closure);
}

/* Constructor for channel_data */
static grpc_error_handle connected_channel_init_channel_elem(
    grpc_channel_element* elem, grpc_channel_element_args* args) {
  channel_data* cd = static_cast<channel_data*>(elem->channel_data);
  GPR_ASSERT(args->is_last);
  cd->transport = grpc_channel_args_find_pointer<grpc_transport>(
      args->channel_args, GRPC_ARG_TRANSPORT);
  return absl::OkStatus();
}

/* Destructor for channel_data */
static void connected_channel_destroy_channel_elem(grpc_channel_element* elem) {
  channel_data* cd = static_cast<channel_data*>(elem->channel_data);
  if (cd->transport) {
    grpc_transport_destroy(cd->transport);
  }
}

/* No-op. */
static void connected_channel_get_channel_info(
    grpc_channel_element* /*elem*/, const grpc_channel_info* /*channel_info*/) {
}

namespace grpc_core {
namespace {

class ConnectedChannelStream : public Orphanable {
 public:
  grpc_transport* transport() { return transport_; }
  grpc_closure* stream_destroyed_closure() { return &stream_destroyed_; }

  void IncrementRefCount(const char* reason) {
#ifndef NDEBUG
    grpc_stream_ref(&stream_refcount_, reason);
#else
    (void)reason;
    grpc_stream_ref(&stream_refcount_);
#endif
  }

  void Unref(const char* reason) {
#ifndef NDEBUG
    grpc_stream_unref(&stream_refcount_, reason);
#else
    (void)reason;
    grpc_stream_unref(&stream_refcount_);
#endif
  }

 protected:
  explicit ConnectedChannelStream(grpc_transport* transport)
      : transport_(transport), stream_(nullptr, StreamDeleter(this)) {
    call_context_->IncrementRefCount("connected_channel_stream");
    GRPC_STREAM_REF_INIT(
        &stream_refcount_, 1,
        [](void* p, grpc_error_handle) {
          static_cast<ConnectedChannelStream*>(p)->BeginDestroy();
        },
        this, "client_stream");
  }

  grpc_stream* stream() { return stream_.get(); }
  void SetStream(grpc_stream* stream) { stream_.reset(stream); }
  grpc_stream_refcount* stream_refcount() { return &stream_refcount_; }
  Mutex* mu() const ABSL_LOCK_RETURNED(mu_) { return &mu_; }
  grpc_transport_stream_op_batch_payload* batch_payload() {
    return &batch_payload_;
  }

  void SchedulePush(grpc_transport_stream_op_batch* batch)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    if (push_batches_.empty()) {
      IncrementRefCount("push");
      ExecCtx::Run(DEBUG_LOCATION, &push_, absl::OkStatus());
    }
    push_batches_.push_back(batch);
  }

  void PollSendMessage(PipeReceiver<MessageHandle>* outgoing_messages,
                       ClientMetadataHandle* client_trailing_metadata)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    if (absl::holds_alternative<Closed>(send_message_state_)) {
      message_to_send_.reset();
    }
    if (absl::holds_alternative<Idle>(send_message_state_)) {
      message_to_send_.reset();
      send_message_state_ = outgoing_messages->Next();
    }
    if (auto* next = absl::get_if<PipeReceiver<MessageHandle>::NextType>(
            &send_message_state_)) {
      auto r = (*next)();
      if (auto* p = absl::get_if<NextResult<MessageHandle>>(&r)) {
        memset(&send_message_, 0, sizeof(send_message_));
        send_message_.payload = batch_payload();
        send_message_.on_complete = &send_message_batch_done_;
        // No value => half close from above.
        if (p->has_value()) {
          message_to_send_ = std::move(**p);
          send_message_state_ = SendMessageToTransport{};
          send_message_.send_message = true;
          batch_payload()->send_message.send_message =
              message_to_send_->payload();
          batch_payload()->send_message.flags = message_to_send_->flags();
        } else {
          if (grpc_call_trace.enabled()) {
            gpr_log(GPR_INFO, "%sPollConnectedChannel: half close",
                    Activity::current()->DebugTag().c_str());
          }
          GPR_ASSERT(!absl::holds_alternative<Closed>(send_message_state_));
          send_message_state_ = Closed{};
          send_message_.send_trailing_metadata = true;
          if (client_trailing_metadata != nullptr) {
            *client_trailing_metadata =
                GetContext<Arena>()->MakePooled<ClientMetadata>(
                    GetContext<Arena>());
            batch_payload()->send_trailing_metadata.send_trailing_metadata =
                client_trailing_metadata->get();
            batch_payload()->send_trailing_metadata.sent = nullptr;
          } else {
            return;  // Skip rest of function for server
          }
        }
        IncrementRefCount("send_message");
        send_message_waker_ = Activity::current()->MakeOwningWaker();
        SchedulePush(&send_message_);
      }
    }
  }

  std::string SendMessageString() const ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu()) {
    return Match(
        send_message_state_, [](Idle) -> std::string { return "IDLE"; },
        [](Closed) -> std::string { return "CLOSED"; },
        [](const PipeReceiver<MessageHandle>::NextType&) -> std::string {
          return "WAITING";
        },
        [](SendMessageToTransport) -> std::string { return "SENDING"; });
  }

  // TODO(ctiller): move to private
  struct Idle {};
  struct Closed {};

 private:
  struct SendMessageToTransport {};

  class StreamDeleter {
   public:
    explicit StreamDeleter(ConnectedChannelStream* impl) : impl_(impl) {}
    void operator()(grpc_stream* stream) const {
      if (stream == nullptr) return;
      grpc_transport_destroy_stream(impl_->transport(), stream,
                                    impl_->stream_destroyed_closure());
    }

   private:
    ConnectedChannelStream* impl_;
  };
  using StreamPtr = std::unique_ptr<grpc_stream, StreamDeleter>;

  void StreamDestroyed() {
    call_context_->RunInContext([this] {
      auto* cc = call_context_;
      this->~ConnectedChannelStream();
      cc->Unref("child_stream");
    });
  }

  void BeginDestroy() {
    if (stream_ != nullptr) {
      stream_.reset();
    } else {
      StreamDestroyed();
    }
  }

  // Called from outside the activity to push work down to the transport.
  void Push() {
    PushBatches push_batches;
    {
      MutexLock lock(&mu_);
      push_batches.swap(push_batches_);
    }
    for (auto* batch : push_batches) {
      if (stream() != nullptr) {
        grpc_transport_perform_stream_op(transport(), stream(), batch);
      } else {
        grpc_transport_stream_op_batch_finish_with_failure_from_transport(
            batch, absl::CancelledError());
      }
    }
    Unref("push");
  }

  void SendMessageBatchDone(grpc_error_handle error) {
    {
      MutexLock lock(&mu_);
      if (error != absl::OkStatus()) {
        // Note that we're in error here, the call will be closed by the
        // transport in a moment, and we'll return from the promise with an
        // error - so we don't need to do any extra work to close out pipes or
        // the like.
        send_message_state_ = Closed{};
      }
      if (!absl::holds_alternative<Closed>(send_message_state_)) {
        send_message_state_ = Idle{};
      }
      send_message_waker_.Wakeup();
    }
    Unref("send_message");
  }

  mutable Mutex mu_;
  grpc_transport* const transport_;
  CallContext* const call_context_{GetContext<CallContext>()};
  grpc_closure stream_destroyed_ =
      MakeMemberClosure<ConnectedChannelStream,
                        &ConnectedChannelStream::StreamDestroyed>(
          this, DEBUG_LOCATION);
  grpc_stream_refcount stream_refcount_;
  StreamPtr stream_;
  using PushBatches = absl::InlinedVector<grpc_transport_stream_op_batch*, 3>;
  PushBatches push_batches_ ABSL_GUARDED_BY(mu_);
  grpc_closure push_ =
      MakeMemberClosure<ConnectedChannelStream, &ConnectedChannelStream::Push>(
          this, DEBUG_LOCATION);

  MessageHandle message_to_send_ ABSL_GUARDED_BY(mu_);
  absl::variant<Idle, Closed, PipeReceiver<MessageHandle>::NextType,
                SendMessageToTransport>
      send_message_state_ ABSL_GUARDED_BY(mu_);
  grpc_transport_stream_op_batch send_message_;
  grpc_closure send_message_batch_done_ =
      MakeMemberClosure<ConnectedChannelStream,
                        &ConnectedChannelStream::SendMessageBatchDone>(
          this, DEBUG_LOCATION);

  Waker send_message_waker_ ABSL_GUARDED_BY(mu_);

  grpc_transport_stream_op_batch_payload batch_payload_{
      GetContext<grpc_call_context_element>()};
};

class ClientStream : public ConnectedChannelStream {
 public:
  ClientStream(grpc_transport* transport, CallArgs call_args)
      : ConnectedChannelStream(transport),
        server_initial_metadata_latch_(call_args.server_initial_metadata),
        client_to_server_messages_(call_args.client_to_server_messages),
        server_to_client_messages_(call_args.server_to_client_messages),
        client_initial_metadata_(std::move(call_args.client_initial_metadata)) {
    if (grpc_call_trace.enabled()) {
      gpr_log(GPR_INFO, "%sInitImpl: intitial_metadata=%s",
              Activity::current()->DebugTag().c_str(),
              client_initial_metadata_->DebugString().c_str());
    }
  }

  void Orphan() override {
    bool finished;
    {
      MutexLock lock(mu());
      if (grpc_call_trace.enabled()) {
        gpr_log(GPR_INFO, "%sDropStream: %s",
                Activity::current()->DebugTag().c_str(),
                ActiveOpsString().c_str());
      }
      finished = finished_;
    }
    // If we hadn't already observed the stream to be finished, we need to
    // cancel it at the transport.
    if (!finished) {
      IncrementRefCount("shutdown client stream");
      auto* cancel_op =
          GetContext<Arena>()->New<grpc_transport_stream_op_batch>();
      cancel_op->cancel_stream = true;
      cancel_op->payload = batch_payload();
      auto* s = stream();
      cancel_op->on_complete = NewClosure(
          [this](grpc_error_handle) { Unref("shutdown client stream"); });
      batch_payload()->cancel_stream.cancel_error = absl::CancelledError();
      grpc_transport_perform_stream_op(transport(), s, cancel_op);
    }
    Unref("orphan client stream");
  }

  Poll<ServerMetadataHandle> PollOnce() {
    MutexLock lock(mu());
    GPR_ASSERT(!finished_);

    if (grpc_call_trace.enabled()) {
      gpr_log(GPR_INFO, "%sPollConnectedChannel: %s",
              Activity::current()->DebugTag().c_str(),
              ActiveOpsString().c_str());
    }

    auto push_recv_message = [this]() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu()) {
      recv_message_state_ = PendingReceiveMessage{};
      auto& pending_recv_message =
          absl::get<PendingReceiveMessage>(recv_message_state_);
      memset(&recv_message_, 0, sizeof(recv_message_));
      recv_message_.payload = batch_payload();
      recv_message_.on_complete = nullptr;
      recv_message_.recv_message = true;
      batch_payload()->recv_message.recv_message =
          &pending_recv_message.payload;
      batch_payload()->recv_message.flags = &pending_recv_message.flags;
      batch_payload()->recv_message.call_failed_before_recv_message = nullptr;
      batch_payload()->recv_message.recv_message_ready =
          &recv_message_batch_done_;
      IncrementRefCount("recv_message");
      recv_message_waker_ = Activity::current()->MakeOwningWaker();
      SchedulePush(&recv_message_);
    };

    if (!std::exchange(requested_metadata_, true)) {
      if (grpc_call_trace.enabled()) {
        gpr_log(GPR_INFO, "%sPollConnectedChannel: requesting metadata",
                Activity::current()->DebugTag().c_str());
      }
      SetStream(static_cast<grpc_stream*>(
          GetContext<Arena>()->Alloc(transport()->vtable->sizeof_stream)));
      grpc_transport_init_stream(transport(), stream(), stream_refcount(),
                                 nullptr, GetContext<Arena>());
      grpc_transport_set_pops(transport(), stream(),
                              GetContext<CallContext>()->polling_entity());
      memset(&metadata_, 0, sizeof(metadata_));
      metadata_.send_initial_metadata = true;
      metadata_.recv_initial_metadata = true;
      metadata_.recv_trailing_metadata = true;
      metadata_.payload = batch_payload();
      metadata_.on_complete = &metadata_batch_done_;
      batch_payload()->send_initial_metadata.send_initial_metadata =
          client_initial_metadata_.get();
      batch_payload()->send_initial_metadata.peer_string =
          GetContext<CallContext>()->peer_string_atm_ptr();
      server_initial_metadata_ =
          GetContext<Arena>()->MakePooled<ServerMetadata>(GetContext<Arena>());
      batch_payload()->recv_initial_metadata.recv_initial_metadata =
          server_initial_metadata_.get();
      batch_payload()->recv_initial_metadata.recv_initial_metadata_ready =
          &recv_initial_metadata_ready_;
      batch_payload()->recv_initial_metadata.trailing_metadata_available =
          nullptr;
      batch_payload()->recv_initial_metadata.peer_string = nullptr;
      server_trailing_metadata_ =
          GetContext<Arena>()->MakePooled<ServerMetadata>(GetContext<Arena>());
      batch_payload()->recv_trailing_metadata.recv_trailing_metadata =
          server_trailing_metadata_.get();
      batch_payload()->recv_trailing_metadata.collect_stats =
          &GetContext<CallContext>()->call_stats()->transport_stream_stats;
      batch_payload()->recv_trailing_metadata.recv_trailing_metadata_ready =
          &recv_trailing_metadata_ready_;
      IncrementRefCount("metadata_batch_done");
      IncrementRefCount("initial_metadata_ready");
      IncrementRefCount("trailing_metadata_ready");
      initial_metadata_waker_ = Activity::current()->MakeOwningWaker();
      trailing_metadata_waker_ = Activity::current()->MakeOwningWaker();
      SchedulePush(&metadata_);
    }
    PollSendMessage(client_to_server_messages_, &client_trailing_metadata_);
    if (auto* pending =
            absl::get_if<PendingReceiveMessage>(&recv_message_state_)) {
      if (pending->received) {
        if (pending->payload.has_value()) {
          if (grpc_call_trace.enabled()) {
            gpr_log(GPR_INFO,
                    "%sRecvMessageBatchDone: received payload of %" PRIdPTR
                    " bytes",
                    recv_message_waker_.ActivityDebugTag().c_str(),
                    pending->payload->Length());
          }
          recv_message_state_ = server_to_client_messages_->Push(
              GetContext<Arena>()->MakePooled<Message>(
                  std::move(*pending->payload), pending->flags));
        } else {
          if (grpc_call_trace.enabled()) {
            gpr_log(GPR_INFO, "%sRecvMessageBatchDone: received no payload",
                    recv_message_waker_.ActivityDebugTag().c_str());
          }
          recv_message_state_ = Closed{};
          std::exchange(server_to_client_messages_, nullptr)->Close();
        }
      }
    }
    if (server_initial_metadata_state_ ==
        ServerInitialMetadataState::kReceivedButNotSet) {
      server_initial_metadata_state_ = ServerInitialMetadataState::kSet;
      server_initial_metadata_latch_->Set(server_initial_metadata_.get());
    }
    if (absl::holds_alternative<Idle>(recv_message_state_)) {
      if (grpc_call_trace.enabled()) {
        gpr_log(GPR_INFO, "%sPollConnectedChannel: requesting message",
                Activity::current()->DebugTag().c_str());
      }
      push_recv_message();
    }
    if (server_initial_metadata_state_ == ServerInitialMetadataState::kSet &&
        !absl::holds_alternative<PipeSender<MessageHandle>::PushType>(
            recv_message_state_) &&
        !absl::holds_alternative<PendingReceiveMessage>(recv_message_state_) &&
        std::exchange(queued_trailing_metadata_, false)) {
      if (grpc_call_trace.enabled()) {
        gpr_log(GPR_INFO,
                "%sPollConnectedChannel: finished request, returning: {%s}; "
                "active_ops: %s",
                Activity::current()->DebugTag().c_str(),
                server_trailing_metadata_->DebugString().c_str(),
                ActiveOpsString().c_str());
      }
      finished_ = true;
      return ServerMetadataHandle(std::move(server_trailing_metadata_));
    }
    if (auto* push = absl::get_if<PipeSender<MessageHandle>::PushType>(
            &recv_message_state_)) {
      auto r = (*push)();
      if (bool* result = absl::get_if<bool>(&r)) {
        if (*result) {
          if (!finished_) {
            if (grpc_call_trace.enabled()) {
              gpr_log(GPR_INFO,
                      "%sPollConnectedChannel: pushed message; requesting next",
                      Activity::current()->DebugTag().c_str());
            }
            push_recv_message();
          } else {
            if (grpc_call_trace.enabled()) {
              gpr_log(GPR_INFO,
                      "%sPollConnectedChannel: pushed message and finished; "
                      "marking closed",
                      Activity::current()->DebugTag().c_str());
            }
            recv_message_state_ = Closed{};
          }
        } else {
          if (grpc_call_trace.enabled()) {
            gpr_log(GPR_INFO,
                    "%sPollConnectedChannel: failed to push message; marking "
                    "closed",
                    Activity::current()->DebugTag().c_str());
          }
          recv_message_state_ = Closed{};
        }
      }
    }
    return Pending{};
  }

  void RecvInitialMetadataReady(grpc_error_handle error) {
    GPR_ASSERT(error == absl::OkStatus());
    {
      MutexLock lock(mu());
      server_initial_metadata_state_ =
          ServerInitialMetadataState::kReceivedButNotSet;
      initial_metadata_waker_.Wakeup();
    }
    Unref("initial_metadata_ready");
  }

  void RecvTrailingMetadataReady(grpc_error_handle error) {
    GPR_ASSERT(error == absl::OkStatus());
    {
      MutexLock lock(mu());
      queued_trailing_metadata_ = true;
      trailing_metadata_waker_.Wakeup();
    }
    Unref("trailing_metadata_ready");
  }

  void MetadataBatchDone(grpc_error_handle error) {
    GPR_ASSERT(error == absl::OkStatus());
    Unref("metadata_batch_done");
  }

  void RecvMessageBatchDone(grpc_error_handle error) {
    {
      MutexLock lock(mu());
      if (error != absl::OkStatus()) {
        if (grpc_call_trace.enabled()) {
          gpr_log(GPR_INFO, "%sRecvMessageBatchDone: error=%s",
                  recv_message_waker_.ActivityDebugTag().c_str(),
                  StatusToString(error).c_str());
        }
      } else if (absl::holds_alternative<Closed>(recv_message_state_)) {
        if (grpc_call_trace.enabled()) {
          gpr_log(GPR_INFO, "%sRecvMessageBatchDone: already closed, ignoring",
                  recv_message_waker_.ActivityDebugTag().c_str());
        }
      } else {
        if (grpc_call_trace.enabled()) {
          gpr_log(GPR_INFO, "%sRecvMessageBatchDone: received message",
                  recv_message_waker_.ActivityDebugTag().c_str());
        }
        auto pending =
            absl::get_if<PendingReceiveMessage>(&recv_message_state_);
        GPR_ASSERT(pending != nullptr);
        GPR_ASSERT(pending->received == false);
        pending->received = true;
      }
      recv_message_waker_.Wakeup();
    }
    Unref("recv_message");
  }

 private:
  enum class ServerInitialMetadataState : uint8_t {
    // Initial metadata has not been received from the server.
    kNotReceived,
    // Initial metadata has been received from the server via the transport, but
    // has not yet been set on the latch to publish it up the call stack.
    kReceivedButNotSet,
    // Initial metadata has been received from the server via the transport and
    // has been set on the latch to publish it up the call stack.
    kSet,
  };

  std::string ActiveOpsString() const ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu()) {
    std::vector<std::string> ops;
    if (finished_) ops.push_back("FINISHED");
    // Results from transport
    std::vector<std::string> queued;
    if (server_initial_metadata_state_ ==
        ServerInitialMetadataState::kReceivedButNotSet) {
      queued.push_back("initial_metadata");
    }
    if (queued_trailing_metadata_) queued.push_back("trailing_metadata");
    if (!queued.empty()) {
      ops.push_back(absl::StrCat("queued:", absl::StrJoin(queued, ",")));
    }
    // Send message
    std::string send_message_state = SendMessageString();
    if (send_message_state != "WAITING") {
      ops.push_back(absl::StrCat("send_message:", send_message_state));
    }
    // Receive message
    std::string recv_message_state = RecvMessageString();
    if (recv_message_state != "IDLE") {
      ops.push_back(absl::StrCat("recv_message:", recv_message_state));
    }
    return absl::StrJoin(ops, " ");
  }

  std::string RecvMessageString() const ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu()) {
    return Match(
        recv_message_state_, [](Idle) -> std::string { return "IDLE"; },
        [](Closed) -> std::string { return "CLOSED"; },
        [](const PendingReceiveMessage&) -> std::string { return "WAITING"; },
        [](const absl::optional<MessageHandle>& message) -> std::string {
          return absl::StrCat(
              "READY:", message.has_value()
                            ? absl::StrCat((*message)->payload()->Length(), "b")
                            : "EOS");
        },
        [](const PipeSender<MessageHandle>::PushType&) -> std::string {
          return "PUSHING";
        });
  }

  bool requested_metadata_ = false;
  ServerInitialMetadataState server_initial_metadata_state_
      ABSL_GUARDED_BY(mu()) = ServerInitialMetadataState::kNotReceived;
  bool queued_trailing_metadata_ ABSL_GUARDED_BY(mu()) = false;
  bool finished_ ABSL_GUARDED_BY(mu()) = false;
  Waker initial_metadata_waker_ ABSL_GUARDED_BY(mu());
  Waker trailing_metadata_waker_ ABSL_GUARDED_BY(mu());
  Waker recv_message_waker_ ABSL_GUARDED_BY(mu());
  Latch<ServerMetadata*>* server_initial_metadata_latch_;
  PipeReceiver<MessageHandle>* client_to_server_messages_;
  PipeSender<MessageHandle>* server_to_client_messages_;
  struct PendingReceiveMessage {
    absl::optional<SliceBuffer> payload;
    uint32_t flags;
    bool received = false;
  };
  absl::variant<Idle, PendingReceiveMessage, Closed,
                PipeSender<MessageHandle>::PushType>
      recv_message_state_ ABSL_GUARDED_BY(mu());
  grpc_closure recv_initial_metadata_ready_ =
      MakeMemberClosure<ClientStream, &ClientStream::RecvInitialMetadataReady>(
          this, DEBUG_LOCATION);
  grpc_closure recv_trailing_metadata_ready_ =
      MakeMemberClosure<ClientStream, &ClientStream::RecvTrailingMetadataReady>(
          this, DEBUG_LOCATION);
  ClientMetadataHandle client_initial_metadata_;
  ClientMetadataHandle client_trailing_metadata_;
  ServerMetadataHandle server_initial_metadata_;
  ServerMetadataHandle server_trailing_metadata_;
  grpc_transport_stream_op_batch metadata_;
  grpc_closure metadata_batch_done_ =
      MakeMemberClosure<ClientStream, &ClientStream::MetadataBatchDone>(
          this, DEBUG_LOCATION);
  grpc_closure recv_message_batch_done_ =
      MakeMemberClosure<ClientStream, &ClientStream::RecvMessageBatchDone>(
          this, DEBUG_LOCATION);
  grpc_transport_stream_op_batch recv_message_;
};

class ClientConnectedCallPromise {
 public:
  ClientConnectedCallPromise(grpc_transport* transport, CallArgs call_args)
      : impl_(GetContext<Arena>()->New<ClientStream>(transport,
                                                     std::move(call_args))) {}

  ClientConnectedCallPromise(const ClientConnectedCallPromise&) = delete;
  ClientConnectedCallPromise& operator=(const ClientConnectedCallPromise&) =
      delete;
  ClientConnectedCallPromise(ClientConnectedCallPromise&& other) noexcept
      : impl_(std::exchange(other.impl_, nullptr)) {}
  ClientConnectedCallPromise& operator=(
      ClientConnectedCallPromise&& other) noexcept {
    impl_ = std::move(other.impl_);
    return *this;
  }

  static ArenaPromise<ServerMetadataHandle> Make(grpc_transport* transport,
                                                 CallArgs call_args,
                                                 NextPromiseFactory) {
    return ClientConnectedCallPromise(transport, std::move(call_args));
  }

  Poll<ServerMetadataHandle> operator()() { return impl_->PollOnce(); }

 private:
  OrphanablePtr<ClientStream> impl_;
};

class ServerStream final : public ConnectedChannelStream {
 public:
  ServerStream(grpc_transport* transport, CallArgs call_args,
               NextPromiseFactory next_promise_factory)
      : ConnectedChannelStream(transport) {
    SetStream(static_cast<grpc_stream*>(
        GetContext<Arena>()->Alloc(transport->vtable->sizeof_stream)));
    grpc_transport_init_stream(
        transport, stream(), stream_refcount(),
        static_cast<ServerCallContext*>(GetContext<CallContext>())
            ->server_stream_data(),
        GetContext<Arena>());
    grpc_transport_set_pops(transport, stream(),
                            GetContext<CallContext>()->polling_entity());

    auto& gim =
        client_initial_metadata_state_.emplace<GettingInitialMetadata>(this);
    gim.recv_initial_metadata_ready_waker =
        Activity::current()->MakeOwningWaker();
    memset(&gim.recv_initial_metadata, 0, sizeof(gim.recv_initial_metadata));
    gim.recv_initial_metadata.payload = &batch_payload_;
    gim.recv_initial_metadata.on_complete = nullptr;
    gim.recv_initial_metadata.recv_initial_metadata = true;
    gim.next_promise_factory = std::move(next_promise_factory);
    batch_payload_.recv_initial_metadata.recv_initial_metadata =
        gim.client_initial_metadata.get();
    batch_payload_.recv_initial_metadata.recv_initial_metadata_ready =
        &gim.recv_initial_metadata_ready;
    SchedulePush(&gim.recv_initial_metadata);
  }

  void Orphan() override { abort(); }

  Poll<ServerMetadataHandle> Poll() {
    absl::MutexLock lock(mu());
    if (auto* p =
            absl::get_if<GotInitialMetadata>(&client_initial_metadata_state_)) {
      auto* server_to_client = GetContext<Arena>()->New<Pipe<MessageHandle>>();
      auto* client_to_server = GetContext<Arena>()->New<Pipe<MessageHandle>>();
      auto* server_initial_metadata =
          GetContext<Arena>()->New<Latch<ServerMetadata*>>();
      auto promise = p->next_promise_factory(CallArgs{
          std::move(p->client_initial_metadata), server_initial_metadata,
          &client_to_server->receiver, &server_to_client->sender});
      client_initial_metadata_state_.emplace<Running>(
          Running{server_initial_metadata, &client_to_server->sender,
                  &server_to_client->receiver, std::move(promise)});
    }
    if (auto* p = absl::get_if<Running>(&client_initial_metadata_state_)) {
      PollSendMessage(p->outgoing_messages, nullptr);
      auto poll = p->promise();
      if (auto* r = absl::get_if<ServerMetadataHandle>(&poll)) {
        client_initial_metadata_state_.emplace<Uninitialized>();
        return std::move(*r);
      }
    }
    return Pending{};
  }

 private:
  void RecvInitialMetadataReady(absl::Status status) {
    MutexLock lock(mu());
    auto& getting =
        absl::get<GettingInitialMetadata>(client_initial_metadata_state_);
    gpr_log(GPR_DEBUG, "GOT INITIAL METADATA: err=%s %s",
            status.ToString().c_str(),
            getting.client_initial_metadata->DebugString().c_str());
    GotInitialMetadata got{std::move(getting.client_initial_metadata),
                           std::move(getting.next_promise_factory)};
    auto waker = std::move(getting.recv_initial_metadata_ready_waker);
    client_initial_metadata_state_.emplace<GotInitialMetadata>(std::move(got));
    waker.Wakeup();
  }

  struct Uninitialized {};

  struct GettingInitialMetadata {
    explicit GettingInitialMetadata(ServerStream* stream)
        : recv_initial_metadata_ready(
              MakeMemberClosure<ServerStream,
                                &ServerStream::RecvInitialMetadataReady>(
                  stream)) {}
    grpc_transport_stream_op_batch recv_initial_metadata;
    Waker recv_initial_metadata_ready_waker;
    ClientMetadataHandle client_initial_metadata =
        GetContext<Arena>()->MakePooled<ClientMetadata>(GetContext<Arena>());
    grpc_closure recv_initial_metadata_ready;
    NextPromiseFactory next_promise_factory;
  };

  struct GotInitialMetadata {
    ClientMetadataHandle client_initial_metadata;
    NextPromiseFactory next_promise_factory;
  };

  struct Running {
    Latch<ServerMetadata*>* server_initial_metadata;
    PipeSender<MessageHandle>* incoming_messages;
    PipeReceiver<MessageHandle>* outgoing_messages;
    ArenaPromise<ServerMetadataHandle> promise;
  };

  using ClientInitialMetadataState =
      absl::variant<Uninitialized, GettingInitialMetadata, GotInitialMetadata,
                    Running>;
  ClientInitialMetadataState client_initial_metadata_state_
      ABSL_GUARDED_BY(mu()) = Uninitialized{};

  grpc_transport_stream_op_batch_payload batch_payload_{
      GetContext<grpc_call_context_element>()};
};

class ServerConnectedCallPromise {
 public:
  ServerConnectedCallPromise(grpc_transport* transport, CallArgs call_args,
                             NextPromiseFactory next_promise_factory)
      : impl_(GetContext<Arena>()->New<ServerStream>(
            transport, std::move(call_args), std::move(next_promise_factory))) {
  }

  ServerConnectedCallPromise(const ServerConnectedCallPromise&) = delete;
  ServerConnectedCallPromise& operator=(const ServerConnectedCallPromise&) =
      delete;
  ServerConnectedCallPromise(ServerConnectedCallPromise&& other) noexcept
      : impl_(std::exchange(other.impl_, nullptr)) {}
  ServerConnectedCallPromise& operator=(
      ServerConnectedCallPromise&& other) noexcept {
    impl_ = std::move(other.impl_);
    return *this;
  }

  static ArenaPromise<ServerMetadataHandle> Make(grpc_transport* transport,
                                                 CallArgs call_args,
                                                 NextPromiseFactory next) {
    return ServerConnectedCallPromise(transport, std::move(call_args),
                                      std::move(next));
  }

  Poll<ServerMetadataHandle> operator()() { return impl_->Poll(); }

 private:
  OrphanablePtr<ServerStream> impl_;
};

template <ArenaPromise<ServerMetadataHandle> (*make_call_promise)(
    grpc_transport*, CallArgs, NextPromiseFactory)>
grpc_channel_filter MakeConnectedFilter() {
  // Create a vtable that contains both the legacy call methods (for filter
  // stack based calls) and the new promise based method for creating promise
  // based calls (the latter iff make_call_promise != nullptr).
  // In this way the filter can be inserted into either kind of channel stack,
  // and only if all the filters in the stack are promise based will the call
  // be promise based.
  return {
      connected_channel_start_transport_stream_op_batch,
      [](grpc_channel_element* elem, CallArgs call_args,
         NextPromiseFactory next) {
        grpc_transport* transport =
            static_cast<channel_data*>(elem->channel_data)->transport;
        return make_call_promise(transport, std::move(call_args),
                                 std::move(next));
      },
      connected_channel_start_transport_op,
      sizeof(call_data),
      connected_channel_init_call_elem,
      set_pollset_or_pollset_set,
      connected_channel_destroy_call_elem,
      sizeof(channel_data),
      connected_channel_init_channel_elem,
      +[](grpc_channel_stack* channel_stack, grpc_channel_element* elem) {
        /* HACK(ctiller): increase call stack size for the channel to make space
           for channel data. We need a cleaner (but performant) way to do this,
           and I'm not sure what that is yet.
           This is only "safe" because call stacks place no additional data
           after the last call element, and the last call element MUST be the
           connected channel. */
        channel_stack->call_stack_size += grpc_transport_stream_size(
            static_cast<channel_data*>(elem->channel_data)->transport);
      },
      connected_channel_destroy_channel_elem,
      connected_channel_get_channel_info,
      "connected",
  };
}

ArenaPromise<ServerMetadataHandle> MakeTransportCallPromise(
    grpc_transport* transport, CallArgs call_args, NextPromiseFactory) {
  return transport->vtable->make_call_promise(transport, std::move(call_args));
}

const grpc_channel_filter kPromiseBasedTransportFilter =
    MakeConnectedFilter<MakeTransportCallPromise>();

const grpc_channel_filter kClientEmulatedFilter =
    MakeConnectedFilter<ClientConnectedCallPromise::Make>();

const grpc_channel_filter kServerEmulatedFilter =
    MakeConnectedFilter<ServerConnectedCallPromise::Make>();

}  // namespace
}  // namespace grpc_core

bool grpc_add_connected_filter(grpc_core::ChannelStackBuilder* builder) {
  grpc_transport* t = builder->transport();
  GPR_ASSERT(t != nullptr);
  // Choose the right vtable for the connected filter.
  // We can't know promise based call or not here (that decision needs the
  // collaboration of all of the filters on the channel, and we don't want
  // ordering constraints on when we add filters).
  // We can know if this results in a promise based call how we'll create our
  // promise (if indeed we can), and so that is the choice made here.
  if (t->vtable->make_call_promise != nullptr) {
    // Option 1, and our ideal: the transport supports promise based calls, and
    // so we simply use the transport directly.
    builder->AppendFilter(&grpc_core::kPromiseBasedTransportFilter);
  } else if (grpc_channel_stack_type_is_client(builder->channel_stack_type())) {
    // Option 2: the transport does not support promise based calls, but we're
    // on the client and so we have an implementation that we can use to convert
    // to batches.
    builder->AppendFilter(&grpc_core::kClientEmulatedFilter);
  } else {
    // Option 3: the transport does not support promise based calls, and we're
    // on the server so we use the server filter.
    builder->AppendFilter(&grpc_core::kServerEmulatedFilter);
  }
  return true;
}
