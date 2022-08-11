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

#include "src/core/lib/surface/call.h"

#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>

#include <algorithm>
#include <atomic>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include "absl/base/thread_annotations.h"
#include "absl/container/inlined_vector.h"
#include "absl/meta/type_traits.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "absl/types/variant.h"
#include "absl/utility/utility.h"

#include <grpc/byte_buffer.h>
#include <grpc/compression.h>
#include <grpc/event_engine/event_engine.h>
#include <grpc/grpc.h>
#include <grpc/impl/codegen/gpr_types.h>
#include <grpc/impl/codegen/propagation_bits.h>
#include <grpc/slice.h>
#include <grpc/slice_buffer.h>
#include <grpc/status.h>
#include <grpc/support/alloc.h>
#include <grpc/support/atm.h>
#include <grpc/support/log.h>
#include <grpc/support/string_util.h>

#include "src/core/lib/channel/call_finalization.h"
#include "src/core/lib/channel/channel_stack.h"
#include "src/core/lib/channel/channelz.h"
#include "src/core/lib/channel/context.h"
#include "src/core/lib/channel/status_util.h"
#include "src/core/lib/compression/compression_internal.h"
#include "src/core/lib/debug/stats.h"
#include "src/core/lib/event_engine/default_event_engine.h"
#include "src/core/lib/gpr/alloc.h"
#include "src/core/lib/gpr/time_precise.h"
#include "src/core/lib/gpr/useful.h"
#include "src/core/lib/gprpp/bitset.h"
#include "src/core/lib/gprpp/cpp_impl_of.h"
#include "src/core/lib/gprpp/debug_location.h"
#include "src/core/lib/gprpp/global_config_env.h"
#include "src/core/lib/gprpp/ref_counted.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/gprpp/sync.h"
#include "src/core/lib/iomgr/call_combiner.h"
#include "src/core/lib/iomgr/exec_ctx.h"
#include "src/core/lib/iomgr/polling_entity.h"
#include "src/core/lib/profiling/timers.h"
#include "src/core/lib/promise/activity.h"
#include "src/core/lib/promise/arena_promise.h"
#include "src/core/lib/promise/context.h"
#include "src/core/lib/promise/latch.h"
#include "src/core/lib/promise/pipe.h"
#include "src/core/lib/promise/poll.h"
#include "src/core/lib/resource_quota/arena.h"
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/slice/slice_internal.h"
#include "src/core/lib/slice/slice_refcount.h"
#include "src/core/lib/surface/api_trace.h"
#include "src/core/lib/surface/call_test_only.h"
#include "src/core/lib/surface/channel.h"
#include "src/core/lib/surface/completion_queue.h"
#include "src/core/lib/surface/server.h"
#include "src/core/lib/surface/validate_metadata.h"
#include "src/core/lib/transport/call_fragments.h"
#include "src/core/lib/transport/error_utils.h"
#include "src/core/lib/transport/metadata_batch.h"
#include "src/core/lib/transport/transport.h"

grpc_core::TraceFlag grpc_call_error_trace(false, "call_error");
grpc_core::TraceFlag grpc_compression_trace(false, "compression");
grpc_core::TraceFlag grpc_call_trace(false, "call");
grpc_core::TraceFlag grpc_call_refcount_trace(false, "call_refcount");

GPR_GLOBAL_CONFIG_DEFINE_BOOL(grpc_experimental_enable_promise_based_call, true,
                              "enable promise based calls");

namespace grpc_core {

///////////////////////////////////////////////////////////////////////////////
// Call

class Call : public CppImplOf<Call, grpc_call> {
 public:
  Arena* arena() { return arena_; }
  bool is_client() const { return is_client_; }

  virtual void ContextSet(grpc_context_index elem, void* value,
                          void (*destroy)(void* value)) = 0;
  virtual void* ContextGet(grpc_context_index elem) const = 0;
  virtual bool Completed() = 0;
  void CancelWithStatus(grpc_status_code status, const char* description);
  virtual void CancelWithError(grpc_error_handle error) = 0;
  virtual void SetCompletionQueue(grpc_completion_queue* cq) = 0;
  char* GetPeer();
  virtual grpc_call_error StartBatch(const grpc_op* ops, size_t nops,
                                     void* notify_tag,
                                     bool is_notify_tag_closure) = 0;
  virtual bool failed_before_recv_message() const = 0;
  virtual bool is_trailers_only() const = 0;
  virtual absl::string_view GetServerAuthority() const = 0;
  virtual void ExternalRef() = 0;
  virtual void ExternalUnref() = 0;
  virtual void InternalRef(const char* reason) = 0;
  virtual void InternalUnref(const char* reason) = 0;

  virtual grpc_compression_algorithm test_only_compression_algorithm() = 0;
  virtual uint32_t test_only_message_flags() = 0;
  virtual uint32_t test_only_encodings_accepted_by_peer() = 0;
  virtual grpc_compression_algorithm compression_for_level(
      grpc_compression_level level) = 0;

  // This should return nullptr for the promise stack (and alternative means
  // for that functionality be invented)
  virtual grpc_call_stack* call_stack() = 0;

  gpr_atm* peer_string_atm_ptr() { return &peer_string_; }

 protected:
  // The maximum number of concurrent batches possible.
  // Based upon the maximum number of individually queueable ops in the batch
  // api:
  //    - initial metadata send
  //    - message send
  //    - status/close send (depending on client/server)
  //    - initial metadata recv
  //    - message recv
  //    - status/close recv (depending on client/server)
  static constexpr size_t kMaxConcurrentBatches = 6;

  struct ParentCall {
    Mutex child_list_mu;
    Call* first_child ABSL_GUARDED_BY(child_list_mu) = nullptr;
  };

  struct ChildCall {
    explicit ChildCall(Call* parent) : parent(parent) {}
    Call* parent;
    /** siblings: children of the same parent form a list, and this list is
       protected under
        parent->mu */
    Call* sibling_next = nullptr;
    Call* sibling_prev = nullptr;
  };

  Call(Arena* arena, bool is_client, Timestamp send_deadline,
       RefCountedPtr<Channel> channel)
      : channel_(channel),
        arena_(arena),
        send_deadline_(send_deadline),
        is_client_(is_client) {
    GPR_DEBUG_ASSERT(arena_ != nullptr);
    GPR_DEBUG_ASSERT(channel_ != nullptr);
  }
  virtual ~Call() = default;

  void DeleteThis();

  ParentCall* GetOrCreateParentCall();
  ParentCall* parent_call();
  Channel* channel() {
    GPR_DEBUG_ASSERT(channel_ != nullptr);
    return channel_.get();
  }

  absl::Status InitParent(Call* parent, uint32_t propagation_mask);
  void PublishToParent(Call* parent);
  void MaybeUnpublishFromParent();
  void PropagateCancellationToChildren();

  Timestamp send_deadline() const { return send_deadline_; }
  void set_send_deadline(Timestamp send_deadline) {
    send_deadline_ = send_deadline;
  }

 private:
  RefCountedPtr<Channel> channel_;
  Arena* const arena_;
  std::atomic<ParentCall*> parent_call_{nullptr};
  ChildCall* child_ = nullptr;
  Timestamp send_deadline_;
  const bool is_client_;
  // flag indicating that cancellation is inherited
  bool cancellation_is_inherited_ = false;
  // A char* indicating the peer name.
  gpr_atm peer_string_ = 0;
};

Call::ParentCall* Call::GetOrCreateParentCall() {
  ParentCall* p = parent_call_.load(std::memory_order_acquire);
  if (p == nullptr) {
    p = arena_->New<ParentCall>();
    ParentCall* expected = nullptr;
    if (!parent_call_.compare_exchange_strong(expected, p,
                                              std::memory_order_release,
                                              std::memory_order_relaxed)) {
      p->~ParentCall();
      p = expected;
    }
  }
  return p;
}

Call::ParentCall* Call::parent_call() {
  return parent_call_.load(std::memory_order_acquire);
}

absl::Status Call::InitParent(Call* parent, uint32_t propagation_mask) {
  child_ = arena()->New<ChildCall>(parent);

  parent->InternalRef("child");
  GPR_ASSERT(is_client_);
  GPR_ASSERT(!parent->is_client_);

  if (propagation_mask & GRPC_PROPAGATE_DEADLINE) {
    send_deadline_ = std::min(send_deadline_, parent->send_deadline_);
  }
  /* for now GRPC_PROPAGATE_TRACING_CONTEXT *MUST* be passed with
   * GRPC_PROPAGATE_STATS_CONTEXT */
  /* TODO(ctiller): This should change to use the appropriate census start_op
   * call. */
  if (propagation_mask & GRPC_PROPAGATE_CENSUS_TRACING_CONTEXT) {
    if (0 == (propagation_mask & GRPC_PROPAGATE_CENSUS_STATS_CONTEXT)) {
      return absl::UnknownError(
          "Census tracing propagation requested without Census context "
          "propagation");
    }
    ContextSet(GRPC_CONTEXT_TRACING, parent->ContextGet(GRPC_CONTEXT_TRACING),
               nullptr);
  } else if (propagation_mask & GRPC_PROPAGATE_CENSUS_STATS_CONTEXT) {
    return absl::UnknownError(
        "Census context propagation requested without Census tracing "
        "propagation");
  }
  if (propagation_mask & GRPC_PROPAGATE_CANCELLATION) {
    cancellation_is_inherited_ = true;
  }
  return absl::OkStatus();
}

void Call::PublishToParent(Call* parent) {
  ChildCall* cc = child_;
  ParentCall* pc = parent->GetOrCreateParentCall();
  MutexLock lock(&pc->child_list_mu);
  if (pc->first_child == nullptr) {
    pc->first_child = this;
    cc->sibling_next = cc->sibling_prev = this;
  } else {
    cc->sibling_next = pc->first_child;
    cc->sibling_prev = pc->first_child->child_->sibling_prev;
    cc->sibling_next->child_->sibling_prev =
        cc->sibling_prev->child_->sibling_next = this;
  }
  if (parent->Completed()) {
    CancelWithError(GRPC_ERROR_CANCELLED);
  }
}

void Call::MaybeUnpublishFromParent() {
  ChildCall* cc = child_;
  if (cc == nullptr) return;

  ParentCall* pc = cc->parent->parent_call();
  {
    MutexLock lock(&pc->child_list_mu);
    if (this == pc->first_child) {
      pc->first_child = cc->sibling_next;
      if (this == pc->first_child) {
        pc->first_child = nullptr;
      }
    }
    cc->sibling_prev->child_->sibling_next = cc->sibling_next;
    cc->sibling_next->child_->sibling_prev = cc->sibling_prev;
  }
  cc->parent->InternalUnref("child");
}

void Call::CancelWithStatus(grpc_status_code status, const char* description) {
  // copying 'description' is needed to ensure the grpc_call_cancel_with_status
  // guarantee that can be short-lived.
  CancelWithError(grpc_error_set_int(
      grpc_error_set_str(GRPC_ERROR_CREATE_FROM_COPIED_STRING(description),
                         GRPC_ERROR_STR_GRPC_MESSAGE, description),
      GRPC_ERROR_INT_GRPC_STATUS, status));
}

void Call::PropagateCancellationToChildren() {
  ParentCall* pc = parent_call();
  if (pc != nullptr) {
    Call* child;
    MutexLock lock(&pc->child_list_mu);
    child = pc->first_child;
    if (child != nullptr) {
      do {
        Call* next_child_call = child->child_->sibling_next;
        if (child->cancellation_is_inherited_) {
          child->InternalRef("propagate_cancel");
          child->CancelWithError(GRPC_ERROR_CANCELLED);
          child->InternalUnref("propagate_cancel");
        }
        child = next_child_call;
      } while (child != pc->first_child);
    }
  }
}

char* Call::GetPeer() {
  char* peer_string = reinterpret_cast<char*>(gpr_atm_acq_load(&peer_string_));
  if (peer_string != nullptr) return gpr_strdup(peer_string);
  peer_string = grpc_channel_get_target(channel_->c_ptr());
  if (peer_string != nullptr) return peer_string;
  return gpr_strdup("unknown");
}

void Call::DeleteThis() {
  RefCountedPtr<Channel> channel = std::move(channel_);
  Arena* arena = arena_;
  this->~Call();
  channel->UpdateCallSizeEstimate(arena->Destroy());
}

///////////////////////////////////////////////////////////////////////////////
// FilterStackCall
// To be removed once promise conversion is complete

class FilterStackCall final : public Call {
 public:
  ~FilterStackCall() override {
    for (int i = 0; i < GRPC_CONTEXT_COUNT; ++i) {
      if (context_[i].destroy) {
        context_[i].destroy(context_[i].value);
      }
    }
    gpr_free(static_cast<void*>(const_cast<char*>(final_info_.error_string)));
  }

  bool Completed() override {
    return gpr_atm_acq_load(&received_final_op_atm_) != 0;
  }

  // TODO(ctiller): return absl::StatusOr<SomeSmartPointer<Call>>?
  static grpc_error_handle Create(grpc_call_create_args* args,
                                  grpc_call** out_call);

  static Call* FromTopElem(grpc_call_element* elem) {
    return FromCallStack(grpc_call_stack_from_top_element(elem));
  }

  grpc_call_stack* call_stack() override {
    return reinterpret_cast<grpc_call_stack*>(
        reinterpret_cast<char*>(this) +
        GPR_ROUND_UP_TO_ALIGNMENT_SIZE(sizeof(*this)));
  }

  grpc_call_element* call_elem(size_t idx) {
    return grpc_call_stack_element(call_stack(), idx);
  }

  CallCombiner* call_combiner() { return &call_combiner_; }

  void CancelWithError(grpc_error_handle error) override;
  void SetCompletionQueue(grpc_completion_queue* cq) override;
  grpc_call_error StartBatch(const grpc_op* ops, size_t nops, void* notify_tag,
                             bool is_notify_tag_closure) override;
  void ExternalRef() override { ext_ref_.Ref(); }
  void ExternalUnref() override;
  void InternalRef(const char* reason) override {
    GRPC_CALL_STACK_REF(call_stack(), reason);
  }
  void InternalUnref(const char* reason) override {
    GRPC_CALL_STACK_UNREF(call_stack(), reason);
  }

  void ContextSet(grpc_context_index elem, void* value,
                  void (*destroy)(void* value)) override;
  void* ContextGet(grpc_context_index elem) const override {
    return context_[elem].value;
  }

  grpc_compression_algorithm compression_for_level(
      grpc_compression_level level) override {
    return encodings_accepted_by_peer_.CompressionAlgorithmForLevel(level);
  }

  bool is_trailers_only() const override {
    bool result = is_trailers_only_;
    GPR_DEBUG_ASSERT(!result || recv_initial_metadata_.TransportSize() == 0);
    return result;
  }

  bool failed_before_recv_message() const override {
    return call_failed_before_recv_message_;
  }

  absl::string_view GetServerAuthority() const override {
    const Slice* authority_metadata =
        recv_initial_metadata_.get_pointer(HttpAuthorityMetadata());
    if (authority_metadata == nullptr) return "";
    return authority_metadata->as_string_view();
  }

  grpc_compression_algorithm test_only_compression_algorithm() override {
    return incoming_compression_algorithm_;
  }

  uint32_t test_only_message_flags() override {
    return test_only_last_message_flags_;
  }

  uint32_t test_only_encodings_accepted_by_peer() override {
    return encodings_accepted_by_peer_.ToLegacyBitmask();
  }

  static size_t InitialSizeEstimate() {
    return sizeof(FilterStackCall) +
           sizeof(BatchControl) * kMaxConcurrentBatches;
  }

 private:
  static constexpr gpr_atm kRecvNone = 0;
  static constexpr gpr_atm kRecvInitialMetadataFirst = 1;

  struct BatchControl {
    FilterStackCall* call_ = nullptr;
    grpc_transport_stream_op_batch op_;
    /* Share memory for cq_completion and notify_tag as they are never needed
       simultaneously. Each byte used in this data structure count as six bytes
       per call, so any savings we can make are worthwhile,

       We use notify_tag to determine whether or not to send notification to the
       completion queue. Once we've made that determination, we can reuse the
       memory for cq_completion. */
    union {
      grpc_cq_completion cq_completion;
      struct {
        /* Any given op indicates completion by either (a) calling a closure or
           (b) sending a notification on the call's completion queue.  If
           \a is_closure is true, \a tag indicates a closure to be invoked;
           otherwise, \a tag indicates the tag to be used in the notification to
           be sent to the completion queue. */
        void* tag;
        bool is_closure;
      } notify_tag;
    } completion_data_;
    grpc_closure start_batch_;
    grpc_closure finish_batch_;
    std::atomic<intptr_t> steps_to_complete_{0};
    AtomicError batch_error_;
    void set_num_steps_to_complete(uintptr_t steps) {
      steps_to_complete_.store(steps, std::memory_order_release);
    }
    bool completed_batch_step() {
      return steps_to_complete_.fetch_sub(1, std::memory_order_acq_rel) == 1;
    }

    void PostCompletion();
    void FinishStep();
    void ProcessDataAfterMetadata();
    void ReceivingStreamReady(grpc_error_handle error);
    void ValidateFilteredMetadata();
    void ReceivingInitialMetadataReady(grpc_error_handle error);
    void ReceivingTrailingMetadataReady(grpc_error_handle error);
    void FinishBatch(grpc_error_handle error);
  };

  FilterStackCall(Arena* arena, const grpc_call_create_args& args)
      : Call(arena, args.server_transport_data == nullptr, args.send_deadline,
             args.channel->Ref()),
        cq_(args.cq),
        stream_op_payload_(context_) {}

  static void ReleaseCall(void* call, grpc_error_handle);
  static void DestroyCall(void* call, grpc_error_handle);

  static FilterStackCall* FromCallStack(grpc_call_stack* call_stack) {
    return reinterpret_cast<FilterStackCall*>(
        reinterpret_cast<char*>(call_stack) -
        GPR_ROUND_UP_TO_ALIGNMENT_SIZE(sizeof(FilterStackCall)));
  }

  void ExecuteBatch(grpc_transport_stream_op_batch* batch,
                    grpc_closure* start_batch_closure);
  void SetFinalStatus(grpc_error_handle error);
  BatchControl* ReuseOrAllocateBatchControl(const grpc_op* ops);
  void HandleCompressionAlgorithmDisabled(
      grpc_compression_algorithm compression_algorithm) GPR_ATTRIBUTE_NOINLINE;
  void HandleCompressionAlgorithmNotAccepted(
      grpc_compression_algorithm compression_algorithm) GPR_ATTRIBUTE_NOINLINE;
  bool PrepareApplicationMetadata(size_t count, grpc_metadata* metadata,
                                  bool is_trailing);
  void PublishAppMetadata(grpc_metadata_batch* b, bool is_trailing);
  void RecvInitialFilter(grpc_metadata_batch* b);
  void RecvTrailingFilter(grpc_metadata_batch* b,
                          grpc_error_handle batch_error);

  RefCount ext_ref_;
  CallCombiner call_combiner_;
  grpc_completion_queue* cq_;
  grpc_polling_entity pollent_;
  gpr_cycle_counter start_time_ = gpr_get_cycle_counter();

  /** has grpc_call_unref been called */
  bool destroy_called_ = false;
  // Trailers-only response status
  bool is_trailers_only_ = false;
  /** which ops are in-flight */
  bool sent_initial_metadata_ = false;
  bool sending_message_ = false;
  bool sent_final_op_ = false;
  bool received_initial_metadata_ = false;
  bool receiving_message_ = false;
  bool requested_final_op_ = false;
  gpr_atm received_final_op_atm_ = 0;

  BatchControl* active_batches_[kMaxConcurrentBatches] = {};
  grpc_transport_stream_op_batch_payload stream_op_payload_;

  /* first idx: is_receiving, second idx: is_trailing */
  grpc_metadata_batch send_initial_metadata_{arena()};
  grpc_metadata_batch send_trailing_metadata_{arena()};
  grpc_metadata_batch recv_initial_metadata_{arena()};
  grpc_metadata_batch recv_trailing_metadata_{arena()};

  /* Buffered read metadata waiting to be returned to the application.
     Element 0 is initial metadata, element 1 is trailing metadata. */
  grpc_metadata_array* buffered_metadata_[2] = {};

  /* Call data useful used for reporting. Only valid after the call has
   * completed */
  grpc_call_final_info final_info_;

  /* Compression algorithm for *incoming* data */
  grpc_compression_algorithm incoming_compression_algorithm_ =
      GRPC_COMPRESS_NONE;
  /* Supported encodings (compression algorithms), a bitset.
   * Always support no compression. */
  CompressionAlgorithmSet encodings_accepted_by_peer_{GRPC_COMPRESS_NONE};

  /* Contexts for various subsystems (security, tracing, ...). */
  grpc_call_context_element context_[GRPC_CONTEXT_COUNT] = {};

  SliceBuffer send_slice_buffer_;
  absl::optional<SliceBuffer> receiving_slice_buffer_;
  uint32_t receiving_stream_flags_;

  bool call_failed_before_recv_message_ = false;
  grpc_byte_buffer** receiving_buffer_ = nullptr;
  grpc_slice receiving_slice_ = grpc_empty_slice();
  grpc_closure receiving_stream_ready_;
  grpc_closure receiving_initial_metadata_ready_;
  grpc_closure receiving_trailing_metadata_ready_;
  uint32_t test_only_last_message_flags_ = 0;
  // Status about operation of call
  bool sent_server_trailing_metadata_ = false;
  gpr_atm cancelled_with_error_ = 0;

  grpc_closure release_call_;

  union {
    struct {
      grpc_status_code* status;
      grpc_slice* status_details;
      const char** error_string;
    } client;
    struct {
      int* cancelled;
      // backpointer to owning server if this is a server side call.
      Server* core_server;
    } server;
  } final_op_;
  AtomicError status_error_;

  /* recv_state can contain one of the following values:
     RECV_NONE :                 :  no initial metadata and messages received
     RECV_INITIAL_METADATA_FIRST :  received initial metadata first
     a batch_control*            :  received messages first

                 +------1------RECV_NONE------3-----+
                 |                                  |
                 |                                  |
                 v                                  v
     RECV_INITIAL_METADATA_FIRST        receiving_stream_ready_bctlp
           |           ^                      |           ^
           |           |                      |           |
           +-----2-----+                      +-----4-----+

    For 1, 4: See receiving_initial_metadata_ready() function
    For 2, 3: See receiving_stream_ready() function */
  gpr_atm recv_state_ = 0;
};

grpc_error_handle FilterStackCall::Create(grpc_call_create_args* args,
                                          grpc_call** out_call) {
  GPR_TIMER_SCOPE("grpc_call_create", 0);

  Channel* channel = args->channel.get();

  auto add_init_error = [](grpc_error_handle* composite,
                           grpc_error_handle new_err) {
    if (GRPC_ERROR_IS_NONE(new_err)) return;
    if (GRPC_ERROR_IS_NONE(*composite)) {
      *composite = GRPC_ERROR_CREATE_FROM_STATIC_STRING("Call creation failed");
    }
    *composite = grpc_error_add_child(*composite, new_err);
  };

  Arena* arena;
  FilterStackCall* call;
  grpc_error_handle error = GRPC_ERROR_NONE;
  grpc_channel_stack* channel_stack = channel->channel_stack();
  size_t initial_size = channel->CallSizeEstimate();
  GRPC_STATS_INC_CALL_INITIAL_SIZE(initial_size);
  size_t call_alloc_size =
      GPR_ROUND_UP_TO_ALIGNMENT_SIZE(sizeof(FilterStackCall)) +
      channel_stack->call_stack_size;

  std::pair<Arena*, void*> arena_with_call = Arena::CreateWithAlloc(
      initial_size, call_alloc_size, channel->allocator());
  arena = arena_with_call.first;
  call = new (arena_with_call.second) FilterStackCall(arena, *args);
  GPR_DEBUG_ASSERT(FromC(call->c_ptr()) == call);
  GPR_DEBUG_ASSERT(FromCallStack(call->call_stack()) == call);
  *out_call = call->c_ptr();
  grpc_slice path = grpc_empty_slice();
  if (call->is_client()) {
    call->final_op_.client.status_details = nullptr;
    call->final_op_.client.status = nullptr;
    call->final_op_.client.error_string = nullptr;
    GRPC_STATS_INC_CLIENT_CALLS_CREATED();
    path = grpc_slice_ref_internal(args->path->c_slice());
    call->send_initial_metadata_.Set(HttpPathMetadata(),
                                     std::move(*args->path));
    if (args->authority.has_value()) {
      call->send_initial_metadata_.Set(HttpAuthorityMetadata(),
                                       std::move(*args->authority));
    }
  } else {
    GRPC_STATS_INC_SERVER_CALLS_CREATED();
    call->final_op_.server.cancelled = nullptr;
    call->final_op_.server.core_server = args->server;
  }

  Call* parent = Call::FromC(args->parent);
  if (parent != nullptr) {
    add_init_error(&error, absl_status_to_grpc_error(call->InitParent(
                               parent, args->propagation_mask)));
  }
  /* initial refcount dropped by grpc_call_unref */
  grpc_call_element_args call_args = {
      call->call_stack(), args->server_transport_data,
      call->context_,     path,
      call->start_time_,  call->send_deadline(),
      call->arena(),      &call->call_combiner_};
  add_init_error(&error, grpc_call_stack_init(channel_stack, 1, DestroyCall,
                                              call, &call_args));
  // Publish this call to parent only after the call stack has been initialized.
  if (parent != nullptr) {
    call->PublishToParent(parent);
  }

  if (!GRPC_ERROR_IS_NONE(error)) {
    call->CancelWithError(GRPC_ERROR_REF(error));
  }
  if (args->cq != nullptr) {
    GPR_ASSERT(args->pollset_set_alternative == nullptr &&
               "Only one of 'cq' and 'pollset_set_alternative' should be "
               "non-nullptr.");
    GRPC_CQ_INTERNAL_REF(args->cq, "bind");
    call->pollent_ =
        grpc_polling_entity_create_from_pollset(grpc_cq_pollset(args->cq));
  }
  if (args->pollset_set_alternative != nullptr) {
    call->pollent_ = grpc_polling_entity_create_from_pollset_set(
        args->pollset_set_alternative);
  }
  if (!grpc_polling_entity_is_empty(&call->pollent_)) {
    grpc_call_stack_set_pollset_or_pollset_set(call->call_stack(),
                                               &call->pollent_);
  }

  if (call->is_client()) {
    channelz::ChannelNode* channelz_channel = channel->channelz_node();
    if (channelz_channel != nullptr) {
      channelz_channel->RecordCallStarted();
    }
  } else if (call->final_op_.server.core_server != nullptr) {
    channelz::ServerNode* channelz_node =
        call->final_op_.server.core_server->channelz_node();
    if (channelz_node != nullptr) {
      channelz_node->RecordCallStarted();
    }
  }

  grpc_slice_unref_internal(path);

  return error;
}

void FilterStackCall::SetCompletionQueue(grpc_completion_queue* cq) {
  GPR_ASSERT(cq);

  if (grpc_polling_entity_pollset_set(&pollent_) != nullptr) {
    gpr_log(GPR_ERROR, "A pollset_set is already registered for this call.");
    abort();
  }
  cq_ = cq;
  GRPC_CQ_INTERNAL_REF(cq, "bind");
  pollent_ = grpc_polling_entity_create_from_pollset(grpc_cq_pollset(cq));
  grpc_call_stack_set_pollset_or_pollset_set(call_stack(), &pollent_);
}

void FilterStackCall::ReleaseCall(void* call, grpc_error_handle /*error*/) {
  static_cast<FilterStackCall*>(call)->DeleteThis();
}

void FilterStackCall::DestroyCall(void* call, grpc_error_handle /*error*/) {
  GPR_TIMER_SCOPE("destroy_call", 0);
  auto* c = static_cast<FilterStackCall*>(call);
  c->recv_initial_metadata_.Clear();
  c->recv_trailing_metadata_.Clear();
  c->receiving_slice_buffer_.reset();
  ParentCall* pc = c->parent_call();
  if (pc != nullptr) {
    pc->~ParentCall();
  }
  if (c->cq_) {
    GRPC_CQ_INTERNAL_UNREF(c->cq_, "bind");
  }

  grpc_error_handle status_error = c->status_error_.get();
  grpc_error_get_status(status_error, c->send_deadline(),
                        &c->final_info_.final_status, nullptr, nullptr,
                        &(c->final_info_.error_string));
  c->status_error_.set(GRPC_ERROR_NONE);
  c->final_info_.stats.latency =
      gpr_cycle_counter_sub(gpr_get_cycle_counter(), c->start_time_);
  grpc_call_stack_destroy(c->call_stack(), &c->final_info_,
                          GRPC_CLOSURE_INIT(&c->release_call_, ReleaseCall, c,
                                            grpc_schedule_on_exec_ctx));
}

void FilterStackCall::ExternalUnref() {
  if (GPR_LIKELY(!ext_ref_.Unref())) return;

  GPR_TIMER_SCOPE("grpc_call_unref", 0);

  ApplicationCallbackExecCtx callback_exec_ctx;
  ExecCtx exec_ctx;

  GRPC_API_TRACE("grpc_call_unref(c=%p)", 1, (this));

  MaybeUnpublishFromParent();

  GPR_ASSERT(!destroy_called_);
  destroy_called_ = true;
  bool cancel = gpr_atm_acq_load(&received_final_op_atm_) == 0;
  if (cancel) {
    CancelWithError(GRPC_ERROR_CANCELLED);
  } else {
    // Unset the call combiner cancellation closure.  This has the
    // effect of scheduling the previously set cancellation closure, if
    // any, so that it can release any internal references it may be
    // holding to the call stack.
    call_combiner_.SetNotifyOnCancel(nullptr);
  }
  InternalUnref("destroy");
}

// start_batch_closure points to a caller-allocated closure to be used
// for entering the call combiner.
void FilterStackCall::ExecuteBatch(grpc_transport_stream_op_batch* batch,
                                   grpc_closure* start_batch_closure) {
  // This is called via the call combiner to start sending a batch down
  // the filter stack.
  auto execute_batch_in_call_combiner = [](void* arg, grpc_error_handle) {
    GPR_TIMER_SCOPE("execute_batch_in_call_combiner", 0);
    grpc_transport_stream_op_batch* batch =
        static_cast<grpc_transport_stream_op_batch*>(arg);
    auto* call =
        static_cast<FilterStackCall*>(batch->handler_private.extra_arg);
    grpc_call_element* elem = call->call_elem(0);
    GRPC_CALL_LOG_OP(GPR_INFO, elem, batch);
    elem->filter->start_transport_stream_op_batch(elem, batch);
  };
  batch->handler_private.extra_arg = this;
  GRPC_CLOSURE_INIT(start_batch_closure, execute_batch_in_call_combiner, batch,
                    grpc_schedule_on_exec_ctx);
  GRPC_CALL_COMBINER_START(call_combiner(), start_batch_closure,
                           GRPC_ERROR_NONE, "executing batch");
}

namespace {
struct CancelState {
  FilterStackCall* call;
  grpc_closure start_batch;
  grpc_closure finish_batch;
};
}  // namespace

// The on_complete callback used when sending a cancel_stream batch down
// the filter stack.  Yields the call combiner when the batch is done.
static void done_termination(void* arg, grpc_error_handle /*error*/) {
  CancelState* state = static_cast<CancelState*>(arg);
  GRPC_CALL_COMBINER_STOP(state->call->call_combiner(),
                          "on_complete for cancel_stream op");
  state->call->InternalUnref("termination");
  delete state;
}

void FilterStackCall::CancelWithError(grpc_error_handle error) {
  if (!gpr_atm_rel_cas(&cancelled_with_error_, 0, 1)) {
    GRPC_ERROR_UNREF(error);
    return;
  }
  InternalRef("termination");
  // Inform the call combiner of the cancellation, so that it can cancel
  // any in-flight asynchronous actions that may be holding the call
  // combiner.  This ensures that the cancel_stream batch can be sent
  // down the filter stack in a timely manner.
  call_combiner_.Cancel(GRPC_ERROR_REF(error));
  CancelState* state = new CancelState;
  state->call = this;
  GRPC_CLOSURE_INIT(&state->finish_batch, done_termination, state,
                    grpc_schedule_on_exec_ctx);
  grpc_transport_stream_op_batch* op =
      grpc_make_transport_stream_op(&state->finish_batch);
  op->cancel_stream = true;
  op->payload->cancel_stream.cancel_error = error;
  ExecuteBatch(op, &state->start_batch);
}

void FilterStackCall::SetFinalStatus(grpc_error_handle error) {
  if (GRPC_TRACE_FLAG_ENABLED(grpc_call_error_trace)) {
    gpr_log(GPR_DEBUG, "set_final_status %s", is_client() ? "CLI" : "SVR");
    gpr_log(GPR_DEBUG, "%s", grpc_error_std_string(error).c_str());
  }
  if (is_client()) {
    std::string status_details;
    grpc_error_get_status(error, send_deadline(), final_op_.client.status,
                          &status_details, nullptr,
                          final_op_.client.error_string);
    *final_op_.client.status_details =
        grpc_slice_from_cpp_string(std::move(status_details));
    status_error_.set(error);
    GRPC_ERROR_UNREF(error);
    channelz::ChannelNode* channelz_channel = channel()->channelz_node();
    if (channelz_channel != nullptr) {
      if (*final_op_.client.status != GRPC_STATUS_OK) {
        channelz_channel->RecordCallFailed();
      } else {
        channelz_channel->RecordCallSucceeded();
      }
    }
  } else {
    *final_op_.server.cancelled =
        !GRPC_ERROR_IS_NONE(error) || !sent_server_trailing_metadata_;
    channelz::ServerNode* channelz_node =
        final_op_.server.core_server->channelz_node();
    if (channelz_node != nullptr) {
      if (*final_op_.server.cancelled || !status_error_.ok()) {
        channelz_node->RecordCallFailed();
      } else {
        channelz_node->RecordCallSucceeded();
      }
    }
    GRPC_ERROR_UNREF(error);
  }
}

bool FilterStackCall::PrepareApplicationMetadata(size_t count,
                                                 grpc_metadata* metadata,
                                                 bool is_trailing) {
  grpc_metadata_batch* batch =
      is_trailing ? &send_trailing_metadata_ : &send_initial_metadata_;
  for (size_t i = 0; i < count; i++) {
    grpc_metadata* md = &metadata[i];
    if (!GRPC_LOG_IF_ERROR("validate_metadata",
                           grpc_validate_header_key_is_legal(md->key))) {
      return false;
    } else if (!grpc_is_binary_header_internal(md->key) &&
               !GRPC_LOG_IF_ERROR(
                   "validate_metadata",
                   grpc_validate_header_nonbin_value_is_legal(md->value))) {
      return false;
    } else if (GRPC_SLICE_LENGTH(md->value) >= UINT32_MAX) {
      // HTTP2 hpack encoding has a maximum limit.
      return false;
    } else if (grpc_slice_str_cmp(md->key, "content-length") == 0) {
      // Filter "content-length metadata"
      continue;
    }
    batch->Append(StringViewFromSlice(md->key),
                  Slice(grpc_slice_ref_internal(md->value)),
                  [md](absl::string_view error, const Slice& value) {
                    gpr_log(GPR_DEBUG, "Append error: %s",
                            absl::StrCat("key=", StringViewFromSlice(md->key),
                                         " error=", error,
                                         " value=", value.as_string_view())
                                .c_str());
                  });
  }

  return true;
}

namespace {
class PublishToAppEncoder {
 public:
  explicit PublishToAppEncoder(grpc_metadata_array* dest) : dest_(dest) {}

  void Encode(const Slice& key, const Slice& value) {
    Append(key.c_slice(), value.c_slice());
  }

  // Catch anything that is not explicitly handled, and do not publish it to the
  // application. If new metadata is added to a batch that needs to be
  // published, it should be called out here.
  template <typename Which>
  void Encode(Which, const typename Which::ValueType&) {}

  void Encode(UserAgentMetadata, const Slice& slice) {
    Append(UserAgentMetadata::key(), slice);
  }

  void Encode(HostMetadata, const Slice& slice) {
    Append(HostMetadata::key(), slice);
  }

  void Encode(GrpcPreviousRpcAttemptsMetadata, uint32_t count) {
    Append(GrpcPreviousRpcAttemptsMetadata::key(), count);
  }

  void Encode(GrpcRetryPushbackMsMetadata, Duration count) {
    Append(GrpcRetryPushbackMsMetadata::key(), count.millis());
  }

  void Encode(LbTokenMetadata, const Slice& slice) {
    Append(LbTokenMetadata::key(), slice);
  }

 private:
  void Append(absl::string_view key, int64_t value) {
    Append(StaticSlice::FromStaticString(key).c_slice(),
           Slice::FromInt64(value).c_slice());
  }

  void Append(absl::string_view key, const Slice& value) {
    Append(StaticSlice::FromStaticString(key).c_slice(), value.c_slice());
  }

  void Append(grpc_slice key, grpc_slice value) {
    auto* mdusr = &dest_->metadata[dest_->count++];
    mdusr->key = key;
    mdusr->value = value;
  }

  grpc_metadata_array* const dest_;
};
}  // namespace

void FilterStackCall::PublishAppMetadata(grpc_metadata_batch* b,
                                         bool is_trailing) {
  if (b->count() == 0) return;
  if (!is_client() && is_trailing) return;
  if (is_trailing && buffered_metadata_[1] == nullptr) return;
  GPR_TIMER_SCOPE("publish_app_metadata", 0);
  grpc_metadata_array* dest;
  dest = buffered_metadata_[is_trailing];
  if (dest->count + b->count() > dest->capacity) {
    dest->capacity =
        std::max(dest->capacity + b->count(), dest->capacity * 3 / 2);
    dest->metadata = static_cast<grpc_metadata*>(
        gpr_realloc(dest->metadata, sizeof(grpc_metadata) * dest->capacity));
  }
  PublishToAppEncoder encoder(dest);
  b->Encode(&encoder);
}

void FilterStackCall::RecvInitialFilter(grpc_metadata_batch* b) {
  incoming_compression_algorithm_ =
      b->Take(GrpcEncodingMetadata()).value_or(GRPC_COMPRESS_NONE);
  encodings_accepted_by_peer_ =
      b->Take(GrpcAcceptEncodingMetadata())
          .value_or(CompressionAlgorithmSet{GRPC_COMPRESS_NONE});
  PublishAppMetadata(b, false);
}

void FilterStackCall::RecvTrailingFilter(grpc_metadata_batch* b,
                                         grpc_error_handle batch_error) {
  if (!GRPC_ERROR_IS_NONE(batch_error)) {
    SetFinalStatus(batch_error);
  } else {
    absl::optional<grpc_status_code> grpc_status =
        b->Take(GrpcStatusMetadata());
    if (grpc_status.has_value()) {
      grpc_status_code status_code = *grpc_status;
      grpc_error_handle error = GRPC_ERROR_NONE;
      if (status_code != GRPC_STATUS_OK) {
        char* peer = GetPeer();
        error = grpc_error_set_int(
            GRPC_ERROR_CREATE_FROM_CPP_STRING(
                absl::StrCat("Error received from peer ", peer)),
            GRPC_ERROR_INT_GRPC_STATUS, static_cast<intptr_t>(status_code));
        gpr_free(peer);
      }
      auto grpc_message = b->Take(GrpcMessageMetadata());
      if (grpc_message.has_value()) {
        error = grpc_error_set_str(error, GRPC_ERROR_STR_GRPC_MESSAGE,
                                   grpc_message->as_string_view());
      } else if (!GRPC_ERROR_IS_NONE(error)) {
        error = grpc_error_set_str(error, GRPC_ERROR_STR_GRPC_MESSAGE, "");
      }
      SetFinalStatus(GRPC_ERROR_REF(error));
      GRPC_ERROR_UNREF(error);
    } else if (!is_client()) {
      SetFinalStatus(GRPC_ERROR_NONE);
    } else {
      gpr_log(GPR_DEBUG,
              "Received trailing metadata with no error and no status");
      SetFinalStatus(grpc_error_set_int(
          GRPC_ERROR_CREATE_FROM_STATIC_STRING("No status received"),
          GRPC_ERROR_INT_GRPC_STATUS, GRPC_STATUS_UNKNOWN));
    }
  }
  PublishAppMetadata(b, true);
}

namespace {
bool AreWriteFlagsValid(uint32_t flags) {
  /* check that only bits in GRPC_WRITE_(INTERNAL?)_USED_MASK are set */
  const uint32_t allowed_write_positions =
      (GRPC_WRITE_USED_MASK | GRPC_WRITE_INTERNAL_USED_MASK);
  const uint32_t invalid_positions = ~allowed_write_positions;
  return !(flags & invalid_positions);
}

bool AreInitialMetadataFlagsValid(uint32_t flags) {
  /* check that only bits in GRPC_WRITE_(INTERNAL?)_USED_MASK are set */
  uint32_t invalid_positions = ~GRPC_INITIAL_METADATA_USED_MASK;
  return !(flags & invalid_positions);
}

size_t BatchSlotForOp(grpc_op_type type) {
  switch (type) {
    case GRPC_OP_SEND_INITIAL_METADATA:
      return 0;
    case GRPC_OP_SEND_MESSAGE:
      return 1;
    case GRPC_OP_SEND_CLOSE_FROM_CLIENT:
    case GRPC_OP_SEND_STATUS_FROM_SERVER:
      return 2;
    case GRPC_OP_RECV_INITIAL_METADATA:
      return 3;
    case GRPC_OP_RECV_MESSAGE:
      return 4;
    case GRPC_OP_RECV_CLOSE_ON_SERVER:
    case GRPC_OP_RECV_STATUS_ON_CLIENT:
      return 5;
  }
  GPR_UNREACHABLE_CODE(return 123456789);
}
}  // namespace

FilterStackCall::BatchControl* FilterStackCall::ReuseOrAllocateBatchControl(
    const grpc_op* ops) {
  size_t slot_idx = BatchSlotForOp(ops[0].op);
  BatchControl** pslot = &active_batches_[slot_idx];
  BatchControl* bctl;
  if (*pslot != nullptr) {
    bctl = *pslot;
    if (bctl->call_ != nullptr) {
      return nullptr;
    }
    bctl->~BatchControl();
    bctl->op_ = {};
    new (&bctl->batch_error_) AtomicError();
  } else {
    bctl = arena()->New<BatchControl>();
    *pslot = bctl;
  }
  bctl->call_ = this;
  bctl->op_.payload = &stream_op_payload_;
  return bctl;
}

void FilterStackCall::BatchControl::PostCompletion() {
  FilterStackCall* call = call_;
  grpc_error_handle error = GRPC_ERROR_REF(batch_error_.get());

  if (op_.send_initial_metadata) {
    call->send_initial_metadata_.Clear();
  }
  if (op_.send_message) {
    if (op_.payload->send_message.stream_write_closed &&
        GRPC_ERROR_IS_NONE(error)) {
      error = grpc_error_add_child(
          error, GRPC_ERROR_CREATE_FROM_STATIC_STRING(
                     "Attempt to send message after stream was closed."));
    }
    call->sending_message_ = false;
    call->send_slice_buffer_.Clear();
  }
  if (op_.send_trailing_metadata) {
    call->send_trailing_metadata_.Clear();
  }
  if (op_.recv_trailing_metadata) {
    /* propagate cancellation to any interested children */
    gpr_atm_rel_store(&call->received_final_op_atm_, 1);
    call->PropagateCancellationToChildren();
    GRPC_ERROR_UNREF(error);
    error = GRPC_ERROR_NONE;
  }
  if (!GRPC_ERROR_IS_NONE(error) && op_.recv_message &&
      *call->receiving_buffer_ != nullptr) {
    grpc_byte_buffer_destroy(*call->receiving_buffer_);
    *call->receiving_buffer_ = nullptr;
  }
  batch_error_.set(GRPC_ERROR_NONE);

  if (completion_data_.notify_tag.is_closure) {
    /* unrefs error */
    call_ = nullptr;
    Closure::Run(DEBUG_LOCATION,
                 static_cast<grpc_closure*>(completion_data_.notify_tag.tag),
                 error);
    call->InternalUnref("completion");
  } else {
    /* unrefs error */
    grpc_cq_end_op(
        call->cq_, completion_data_.notify_tag.tag, error,
        [](void* user_data, grpc_cq_completion* /*storage*/) {
          BatchControl* bctl = static_cast<BatchControl*>(user_data);
          Call* call = bctl->call_;
          bctl->call_ = nullptr;
          call->InternalUnref("completion");
        },
        this, &completion_data_.cq_completion);
  }
}

void FilterStackCall::BatchControl::FinishStep() {
  if (GPR_UNLIKELY(completed_batch_step())) {
    PostCompletion();
  }
}

void FilterStackCall::BatchControl::ProcessDataAfterMetadata() {
  FilterStackCall* call = call_;
  if (!call->receiving_slice_buffer_.has_value()) {
    *call->receiving_buffer_ = nullptr;
    call->receiving_message_ = false;
    FinishStep();
  } else {
    call->test_only_last_message_flags_ = call->receiving_stream_flags_;
    if ((call->receiving_stream_flags_ & GRPC_WRITE_INTERNAL_COMPRESS) &&
        (call->incoming_compression_algorithm_ != GRPC_COMPRESS_NONE)) {
      *call->receiving_buffer_ = grpc_raw_compressed_byte_buffer_create(
          nullptr, 0, call->incoming_compression_algorithm_);
    } else {
      *call->receiving_buffer_ = grpc_raw_byte_buffer_create(nullptr, 0);
    }
    grpc_slice_buffer_move_into(
        call->receiving_slice_buffer_->c_slice_buffer(),
        &(*call->receiving_buffer_)->data.raw.slice_buffer);
    call->receiving_message_ = false;
    call->receiving_slice_buffer_.reset();
    FinishStep();
  }
}

void FilterStackCall::BatchControl::ReceivingStreamReady(
    grpc_error_handle error) {
  FilterStackCall* call = call_;
  if (!GRPC_ERROR_IS_NONE(error)) {
    call->receiving_slice_buffer_.reset();
    if (batch_error_.ok()) {
      batch_error_.set(error);
    }
    call->CancelWithError(GRPC_ERROR_REF(error));
  }
  /* If recv_state is kRecvNone, we will save the batch_control
   * object with rel_cas, and will not use it after the cas. Its corresponding
   * acq_load is in receiving_initial_metadata_ready() */
  if (!GRPC_ERROR_IS_NONE(error) ||
      !call->receiving_slice_buffer_.has_value() ||
      !gpr_atm_rel_cas(&call->recv_state_, kRecvNone,
                       reinterpret_cast<gpr_atm>(this))) {
    ProcessDataAfterMetadata();
  }
}

void FilterStackCall::HandleCompressionAlgorithmDisabled(
    grpc_compression_algorithm compression_algorithm) {
  const char* algo_name = nullptr;
  grpc_compression_algorithm_name(compression_algorithm, &algo_name);
  std::string error_msg =
      absl::StrFormat("Compression algorithm '%s' is disabled.", algo_name);
  gpr_log(GPR_ERROR, "%s", error_msg.c_str());
  CancelWithStatus(GRPC_STATUS_UNIMPLEMENTED, error_msg.c_str());
}

void FilterStackCall::HandleCompressionAlgorithmNotAccepted(
    grpc_compression_algorithm compression_algorithm) {
  const char* algo_name = nullptr;
  grpc_compression_algorithm_name(compression_algorithm, &algo_name);
  gpr_log(GPR_ERROR,
          "Compression algorithm ('%s') not present in the "
          "accepted encodings (%s)",
          algo_name,
          std::string(encodings_accepted_by_peer_.ToString()).c_str());
}

void FilterStackCall::BatchControl::ValidateFilteredMetadata() {
  FilterStackCall* call = call_;

  const grpc_compression_options compression_options =
      call->channel()->compression_options();
  const grpc_compression_algorithm compression_algorithm =
      call->incoming_compression_algorithm_;
  if (GPR_UNLIKELY(!CompressionAlgorithmSet::FromUint32(
                        compression_options.enabled_algorithms_bitset)
                        .IsSet(compression_algorithm))) {
    /* check if algorithm is supported by current channel config */
    call->HandleCompressionAlgorithmDisabled(compression_algorithm);
  }
  /* GRPC_COMPRESS_NONE is always set. */
  GPR_DEBUG_ASSERT(call->encodings_accepted_by_peer_.IsSet(GRPC_COMPRESS_NONE));
  if (GPR_UNLIKELY(
          !call->encodings_accepted_by_peer_.IsSet(compression_algorithm))) {
    if (GRPC_TRACE_FLAG_ENABLED(grpc_compression_trace)) {
      call->HandleCompressionAlgorithmNotAccepted(compression_algorithm);
    }
  }
}

void FilterStackCall::BatchControl::ReceivingInitialMetadataReady(
    grpc_error_handle error) {
  FilterStackCall* call = call_;

  GRPC_CALL_COMBINER_STOP(call->call_combiner(), "recv_initial_metadata_ready");

  if (GRPC_ERROR_IS_NONE(error)) {
    grpc_metadata_batch* md = &call->recv_initial_metadata_;
    call->RecvInitialFilter(md);

    /* TODO(ctiller): this could be moved into recv_initial_filter now */
    GPR_TIMER_SCOPE("validate_filtered_metadata", 0);
    ValidateFilteredMetadata();

    absl::optional<Timestamp> deadline = md->get(GrpcTimeoutMetadata());
    if (deadline.has_value() && !call->is_client()) {
      call_->set_send_deadline(*deadline);
    }
  } else {
    if (batch_error_.ok()) {
      batch_error_.set(error);
    }
    call->CancelWithError(GRPC_ERROR_REF(error));
  }

  grpc_closure* saved_rsr_closure = nullptr;
  while (true) {
    gpr_atm rsr_bctlp = gpr_atm_acq_load(&call->recv_state_);
    /* Should only receive initial metadata once */
    GPR_ASSERT(rsr_bctlp != 1);
    if (rsr_bctlp == 0) {
      /* We haven't seen initial metadata and messages before, thus initial
       * metadata is received first.
       * no_barrier_cas is used, as this function won't access the batch_control
       * object saved by receiving_stream_ready() if the initial metadata is
       * received first. */
      if (gpr_atm_no_barrier_cas(&call->recv_state_, kRecvNone,
                                 kRecvInitialMetadataFirst)) {
        break;
      }
    } else {
      /* Already received messages */
      saved_rsr_closure = GRPC_CLOSURE_CREATE(
          [](void* bctl, grpc_error_handle error) {
            static_cast<BatchControl*>(bctl)->ReceivingStreamReady(error);
          },
          reinterpret_cast<BatchControl*>(rsr_bctlp),
          grpc_schedule_on_exec_ctx);
      /* No need to modify recv_state */
      break;
    }
  }
  if (saved_rsr_closure != nullptr) {
    Closure::Run(DEBUG_LOCATION, saved_rsr_closure, GRPC_ERROR_REF(error));
  }

  FinishStep();
}

void FilterStackCall::BatchControl::ReceivingTrailingMetadataReady(
    grpc_error_handle error) {
  GRPC_CALL_COMBINER_STOP(call_->call_combiner(),
                          "recv_trailing_metadata_ready");
  grpc_metadata_batch* md = &call_->recv_trailing_metadata_;
  call_->RecvTrailingFilter(md, GRPC_ERROR_REF(error));
  FinishStep();
}

void FilterStackCall::BatchControl::FinishBatch(grpc_error_handle error) {
  GRPC_CALL_COMBINER_STOP(call_->call_combiner(), "on_complete");
  if (batch_error_.ok()) {
    batch_error_.set(error);
  }
  if (!GRPC_ERROR_IS_NONE(error)) {
    call_->CancelWithError(GRPC_ERROR_REF(error));
  }
  FinishStep();
}

void EndOpImmediately(grpc_completion_queue* cq, void* notify_tag,
                      bool is_notify_tag_closure) {
  if (!is_notify_tag_closure) {
    GPR_ASSERT(grpc_cq_begin_op(cq, notify_tag));
    grpc_cq_end_op(
        cq, notify_tag, GRPC_ERROR_NONE,
        [](void*, grpc_cq_completion* completion) { gpr_free(completion); },
        nullptr,
        static_cast<grpc_cq_completion*>(
            gpr_malloc(sizeof(grpc_cq_completion))));
  } else {
    Closure::Run(DEBUG_LOCATION, static_cast<grpc_closure*>(notify_tag),
                 GRPC_ERROR_NONE);
  }
}

grpc_call_error FilterStackCall::StartBatch(const grpc_op* ops, size_t nops,
                                            void* notify_tag,
                                            bool is_notify_tag_closure) {
  GPR_TIMER_SCOPE("call_start_batch", 0);

  size_t i;
  const grpc_op* op;
  BatchControl* bctl;
  bool has_send_ops = false;
  int num_recv_ops = 0;
  grpc_call_error error = GRPC_CALL_OK;
  grpc_transport_stream_op_batch* stream_op;
  grpc_transport_stream_op_batch_payload* stream_op_payload;
  uint32_t seen_ops = 0;

  for (i = 0; i < nops; i++) {
    if (seen_ops & (1u << ops[i].op)) {
      return GRPC_CALL_ERROR_TOO_MANY_OPERATIONS;
    }
    seen_ops |= (1u << ops[i].op);
  }

  GRPC_CALL_LOG_BATCH(GPR_INFO, ops, nops);

  if (nops == 0) {
    EndOpImmediately(cq_, notify_tag, is_notify_tag_closure);
    error = GRPC_CALL_OK;
    goto done;
  }

  bctl = ReuseOrAllocateBatchControl(ops);
  if (bctl == nullptr) {
    return GRPC_CALL_ERROR_TOO_MANY_OPERATIONS;
  }
  bctl->completion_data_.notify_tag.tag = notify_tag;
  bctl->completion_data_.notify_tag.is_closure =
      static_cast<uint8_t>(is_notify_tag_closure != 0);

  stream_op = &bctl->op_;
  stream_op_payload = &stream_op_payload_;

  /* rewrite batch ops into a transport op */
  for (i = 0; i < nops; i++) {
    op = &ops[i];
    if (op->reserved != nullptr) {
      error = GRPC_CALL_ERROR;
      goto done_with_error;
    }
    switch (op->op) {
      case GRPC_OP_SEND_INITIAL_METADATA: {
        /* Flag validation: currently allow no flags */
        if (!AreInitialMetadataFlagsValid(op->flags)) {
          error = GRPC_CALL_ERROR_INVALID_FLAGS;
          goto done_with_error;
        }
        if (sent_initial_metadata_) {
          error = GRPC_CALL_ERROR_TOO_MANY_OPERATIONS;
          goto done_with_error;
        }
        // TODO(juanlishen): If the user has already specified a compression
        // algorithm by setting the initial metadata with key of
        // GRPC_COMPRESSION_REQUEST_ALGORITHM_MD_KEY, we shouldn't override that
        // with the compression algorithm mapped from compression level.
        /* process compression level */
        grpc_compression_level effective_compression_level =
            GRPC_COMPRESS_LEVEL_NONE;
        bool level_set = false;
        if (op->data.send_initial_metadata.maybe_compression_level.is_set) {
          effective_compression_level =
              op->data.send_initial_metadata.maybe_compression_level.level;
          level_set = true;
        } else {
          const grpc_compression_options copts =
              channel()->compression_options();
          if (copts.default_level.is_set) {
            level_set = true;
            effective_compression_level = copts.default_level.level;
          }
        }
        // Currently, only server side supports compression level setting.
        if (level_set && !is_client()) {
          const grpc_compression_algorithm calgo =
              encodings_accepted_by_peer_.CompressionAlgorithmForLevel(
                  effective_compression_level);
          // The following metadata will be checked and removed by the message
          // compression filter. It will be used as the call's compression
          // algorithm.
          send_initial_metadata_.Set(GrpcInternalEncodingRequest(), calgo);
        }
        if (op->data.send_initial_metadata.count > INT_MAX) {
          error = GRPC_CALL_ERROR_INVALID_METADATA;
          goto done_with_error;
        }
        stream_op->send_initial_metadata = true;
        sent_initial_metadata_ = true;
        if (!PrepareApplicationMetadata(op->data.send_initial_metadata.count,
                                        op->data.send_initial_metadata.metadata,
                                        false)) {
          error = GRPC_CALL_ERROR_INVALID_METADATA;
          goto done_with_error;
        }
        // Ignore any te metadata key value pairs specified.
        send_initial_metadata_.Remove(TeMetadata());
        /* TODO(ctiller): just make these the same variable? */
        if (is_client() && send_deadline() != Timestamp::InfFuture()) {
          send_initial_metadata_.Set(GrpcTimeoutMetadata(), send_deadline());
        }
        if (is_client()) {
          send_initial_metadata_.Set(
              WaitForReady(),
              WaitForReady::ValueType{
                  (op->flags & GRPC_INITIAL_METADATA_WAIT_FOR_READY) != 0,
                  (op->flags &
                   GRPC_INITIAL_METADATA_WAIT_FOR_READY_EXPLICITLY_SET) != 0});
        }
        stream_op_payload->send_initial_metadata.send_initial_metadata =
            &send_initial_metadata_;
        if (is_client()) {
          stream_op_payload->send_initial_metadata.peer_string =
              peer_string_atm_ptr();
        }
        has_send_ops = true;
        break;
      }
      case GRPC_OP_SEND_MESSAGE: {
        if (!AreWriteFlagsValid(op->flags)) {
          error = GRPC_CALL_ERROR_INVALID_FLAGS;
          goto done_with_error;
        }
        if (op->data.send_message.send_message == nullptr) {
          error = GRPC_CALL_ERROR_INVALID_MESSAGE;
          goto done_with_error;
        }
        if (sending_message_) {
          error = GRPC_CALL_ERROR_TOO_MANY_OPERATIONS;
          goto done_with_error;
        }
        uint32_t flags = op->flags;
        /* If the outgoing buffer is already compressed, mark it as so in the
           flags. These will be picked up by the compression filter and further
           (wasteful) attempts at compression skipped. */
        if (op->data.send_message.send_message->data.raw.compression >
            GRPC_COMPRESS_NONE) {
          flags |= GRPC_WRITE_INTERNAL_COMPRESS;
        }
        stream_op->send_message = true;
        sending_message_ = true;
        send_slice_buffer_.Clear();
        grpc_slice_buffer_move_into(
            &op->data.send_message.send_message->data.raw.slice_buffer,
            send_slice_buffer_.c_slice_buffer());
        stream_op_payload->send_message.flags = flags;
        stream_op_payload->send_message.send_message = &send_slice_buffer_;
        has_send_ops = true;
        break;
      }
      case GRPC_OP_SEND_CLOSE_FROM_CLIENT: {
        /* Flag validation: currently allow no flags */
        if (op->flags != 0) {
          error = GRPC_CALL_ERROR_INVALID_FLAGS;
          goto done_with_error;
        }
        if (!is_client()) {
          error = GRPC_CALL_ERROR_NOT_ON_SERVER;
          goto done_with_error;
        }
        if (sent_final_op_) {
          error = GRPC_CALL_ERROR_TOO_MANY_OPERATIONS;
          goto done_with_error;
        }
        stream_op->send_trailing_metadata = true;
        sent_final_op_ = true;
        stream_op_payload->send_trailing_metadata.send_trailing_metadata =
            &send_trailing_metadata_;
        has_send_ops = true;
        break;
      }
      case GRPC_OP_SEND_STATUS_FROM_SERVER: {
        /* Flag validation: currently allow no flags */
        if (op->flags != 0) {
          error = GRPC_CALL_ERROR_INVALID_FLAGS;
          goto done_with_error;
        }
        if (is_client()) {
          error = GRPC_CALL_ERROR_NOT_ON_CLIENT;
          goto done_with_error;
        }
        if (sent_final_op_) {
          error = GRPC_CALL_ERROR_TOO_MANY_OPERATIONS;
          goto done_with_error;
        }
        if (op->data.send_status_from_server.trailing_metadata_count >
            INT_MAX) {
          error = GRPC_CALL_ERROR_INVALID_METADATA;
          goto done_with_error;
        }
        stream_op->send_trailing_metadata = true;
        sent_final_op_ = true;

        if (!PrepareApplicationMetadata(
                op->data.send_status_from_server.trailing_metadata_count,
                op->data.send_status_from_server.trailing_metadata, true)) {
          error = GRPC_CALL_ERROR_INVALID_METADATA;
          goto done_with_error;
        }

        grpc_error_handle status_error =
            op->data.send_status_from_server.status == GRPC_STATUS_OK
                ? GRPC_ERROR_NONE
                : grpc_error_set_int(
                      GRPC_ERROR_CREATE_FROM_STATIC_STRING(
                          "Server returned error"),
                      GRPC_ERROR_INT_GRPC_STATUS,
                      static_cast<intptr_t>(
                          op->data.send_status_from_server.status));
        if (op->data.send_status_from_server.status_details != nullptr) {
          send_trailing_metadata_.Set(
              GrpcMessageMetadata(),
              Slice(grpc_slice_copy(
                  *op->data.send_status_from_server.status_details)));
          if (!GRPC_ERROR_IS_NONE(status_error)) {
            status_error = grpc_error_set_str(
                status_error, GRPC_ERROR_STR_GRPC_MESSAGE,
                StringViewFromSlice(
                    *op->data.send_status_from_server.status_details));
          }
        }

        status_error_.set(status_error);
        GRPC_ERROR_UNREF(status_error);

        send_trailing_metadata_.Set(GrpcStatusMetadata(),
                                    op->data.send_status_from_server.status);

        // Ignore any te metadata key value pairs specified.
        send_trailing_metadata_.Remove(TeMetadata());
        stream_op_payload->send_trailing_metadata.send_trailing_metadata =
            &send_trailing_metadata_;
        stream_op_payload->send_trailing_metadata.sent =
            &sent_server_trailing_metadata_;
        has_send_ops = true;
        break;
      }
      case GRPC_OP_RECV_INITIAL_METADATA: {
        /* Flag validation: currently allow no flags */
        if (op->flags != 0) {
          error = GRPC_CALL_ERROR_INVALID_FLAGS;
          goto done_with_error;
        }
        if (received_initial_metadata_) {
          error = GRPC_CALL_ERROR_TOO_MANY_OPERATIONS;
          goto done_with_error;
        }
        received_initial_metadata_ = true;
        buffered_metadata_[0] =
            op->data.recv_initial_metadata.recv_initial_metadata;
        GRPC_CLOSURE_INIT(
            &receiving_initial_metadata_ready_,
            [](void* bctl, grpc_error_handle error) {
              static_cast<BatchControl*>(bctl)->ReceivingInitialMetadataReady(
                  error);
            },
            bctl, grpc_schedule_on_exec_ctx);
        stream_op->recv_initial_metadata = true;
        stream_op_payload->recv_initial_metadata.recv_initial_metadata =
            &recv_initial_metadata_;
        stream_op_payload->recv_initial_metadata.recv_initial_metadata_ready =
            &receiving_initial_metadata_ready_;
        if (is_client()) {
          stream_op_payload->recv_initial_metadata.trailing_metadata_available =
              &is_trailers_only_;
        } else {
          stream_op_payload->recv_initial_metadata.peer_string =
              peer_string_atm_ptr();
        }
        ++num_recv_ops;
        break;
      }
      case GRPC_OP_RECV_MESSAGE: {
        /* Flag validation: currently allow no flags */
        if (op->flags != 0) {
          error = GRPC_CALL_ERROR_INVALID_FLAGS;
          goto done_with_error;
        }
        if (receiving_message_) {
          error = GRPC_CALL_ERROR_TOO_MANY_OPERATIONS;
          goto done_with_error;
        }
        receiving_message_ = true;
        stream_op->recv_message = true;
        receiving_slice_buffer_.reset();
        receiving_buffer_ = op->data.recv_message.recv_message;
        stream_op_payload->recv_message.recv_message = &receiving_slice_buffer_;
        receiving_stream_flags_ = 0;
        stream_op_payload->recv_message.flags = &receiving_stream_flags_;
        stream_op_payload->recv_message.call_failed_before_recv_message =
            &call_failed_before_recv_message_;
        GRPC_CLOSURE_INIT(
            &receiving_stream_ready_,
            [](void* bctlp, grpc_error_handle error) {
              auto* bctl = static_cast<BatchControl*>(bctlp);
              auto* call = bctl->call_;
              //  Yields the call combiner before processing the received
              //  message.
              GRPC_CALL_COMBINER_STOP(call->call_combiner(),
                                      "recv_message_ready");
              bctl->ReceivingStreamReady(error);
            },
            bctl, grpc_schedule_on_exec_ctx);
        stream_op_payload->recv_message.recv_message_ready =
            &receiving_stream_ready_;
        ++num_recv_ops;
        break;
      }
      case GRPC_OP_RECV_STATUS_ON_CLIENT: {
        /* Flag validation: currently allow no flags */
        if (op->flags != 0) {
          error = GRPC_CALL_ERROR_INVALID_FLAGS;
          goto done_with_error;
        }
        if (!is_client()) {
          error = GRPC_CALL_ERROR_NOT_ON_SERVER;
          goto done_with_error;
        }
        if (requested_final_op_) {
          error = GRPC_CALL_ERROR_TOO_MANY_OPERATIONS;
          goto done_with_error;
        }
        requested_final_op_ = true;
        buffered_metadata_[1] =
            op->data.recv_status_on_client.trailing_metadata;
        final_op_.client.status = op->data.recv_status_on_client.status;
        final_op_.client.status_details =
            op->data.recv_status_on_client.status_details;
        final_op_.client.error_string =
            op->data.recv_status_on_client.error_string;
        stream_op->recv_trailing_metadata = true;
        stream_op_payload->recv_trailing_metadata.recv_trailing_metadata =
            &recv_trailing_metadata_;
        stream_op_payload->recv_trailing_metadata.collect_stats =
            &final_info_.stats.transport_stream_stats;
        GRPC_CLOSURE_INIT(
            &receiving_trailing_metadata_ready_,
            [](void* bctl, grpc_error_handle error) {
              static_cast<BatchControl*>(bctl)->ReceivingTrailingMetadataReady(
                  error);
            },
            bctl, grpc_schedule_on_exec_ctx);
        stream_op_payload->recv_trailing_metadata.recv_trailing_metadata_ready =
            &receiving_trailing_metadata_ready_;
        ++num_recv_ops;
        break;
      }
      case GRPC_OP_RECV_CLOSE_ON_SERVER: {
        /* Flag validation: currently allow no flags */
        if (op->flags != 0) {
          error = GRPC_CALL_ERROR_INVALID_FLAGS;
          goto done_with_error;
        }
        if (is_client()) {
          error = GRPC_CALL_ERROR_NOT_ON_CLIENT;
          goto done_with_error;
        }
        if (requested_final_op_) {
          error = GRPC_CALL_ERROR_TOO_MANY_OPERATIONS;
          goto done_with_error;
        }
        requested_final_op_ = true;
        final_op_.server.cancelled = op->data.recv_close_on_server.cancelled;
        stream_op->recv_trailing_metadata = true;
        stream_op_payload->recv_trailing_metadata.recv_trailing_metadata =
            &recv_trailing_metadata_;
        stream_op_payload->recv_trailing_metadata.collect_stats =
            &final_info_.stats.transport_stream_stats;
        GRPC_CLOSURE_INIT(
            &receiving_trailing_metadata_ready_,
            [](void* bctl, grpc_error_handle error) {
              static_cast<BatchControl*>(bctl)->ReceivingTrailingMetadataReady(
                  error);
            },
            bctl, grpc_schedule_on_exec_ctx);
        stream_op_payload->recv_trailing_metadata.recv_trailing_metadata_ready =
            &receiving_trailing_metadata_ready_;
        ++num_recv_ops;
        break;
      }
    }
  }

  InternalRef("completion");
  if (!is_notify_tag_closure) {
    GPR_ASSERT(grpc_cq_begin_op(cq_, notify_tag));
  }
  bctl->set_num_steps_to_complete((has_send_ops ? 1 : 0) + num_recv_ops);

  if (has_send_ops) {
    GRPC_CLOSURE_INIT(
        &bctl->finish_batch_,
        [](void* bctl, grpc_error_handle error) {
          static_cast<BatchControl*>(bctl)->FinishBatch(error);
        },
        bctl, grpc_schedule_on_exec_ctx);
    stream_op->on_complete = &bctl->finish_batch_;
  }

  ExecuteBatch(stream_op, &bctl->start_batch_);

done:
  return error;

done_with_error:
  /* reverse any mutations that occurred */
  if (stream_op->send_initial_metadata) {
    sent_initial_metadata_ = false;
    send_initial_metadata_.Clear();
  }
  if (stream_op->send_message) {
    sending_message_ = false;
  }
  if (stream_op->send_trailing_metadata) {
    sent_final_op_ = false;
    send_trailing_metadata_.Clear();
  }
  if (stream_op->recv_initial_metadata) {
    received_initial_metadata_ = false;
  }
  if (stream_op->recv_message) {
    receiving_message_ = false;
  }
  if (stream_op->recv_trailing_metadata) {
    requested_final_op_ = false;
  }
  goto done;
}

void FilterStackCall::ContextSet(grpc_context_index elem, void* value,
                                 void (*destroy)(void*)) {
  if (context_[elem].destroy) {
    context_[elem].destroy(context_[elem].value);
  }
  context_[elem].value = value;
  context_[elem].destroy = destroy;
}

///////////////////////////////////////////////////////////////////////////////
// PromiseBasedCall
// Will be folded into Call once the promise conversion is done

class PromiseBasedCall : public Call, public Activity, public Wakeable {
 public:
  PromiseBasedCall(Arena* arena, const grpc_call_create_args& args);

  void ContextSet(grpc_context_index elem, void* value,
                  void (*destroy)(void* value)) override;
  void* ContextGet(grpc_context_index elem) const override;
  void SetCompletionQueue(grpc_completion_queue* cq) override;

  // Implementation of call refcounting: move this to DualRefCounted once we
  // don't need to maintain FilterStackCall compatibility
  void ExternalRef() final {
    refs_.fetch_add(MakeRefPair(1, 0), std::memory_order_relaxed);
  }
  void ExternalUnref() final {
    const uint64_t prev_ref_pair =
        refs_.fetch_add(MakeRefPair(-1, 1), std::memory_order_acq_rel);
    const uint32_t strong_refs = GetStrongRefs(prev_ref_pair);
    if (GPR_UNLIKELY(strong_refs == 1)) {
      Orphan();
    }
    // Now drop the weak ref.
    InternalUnref("external_ref");
  }
  void InternalRef(const char*) final {
    refs_.fetch_add(MakeRefPair(0, 1), std::memory_order_relaxed);
  }
  void InternalUnref(const char*) final {
    const uint64_t prev_ref_pair =
        refs_.fetch_sub(MakeRefPair(0, 1), std::memory_order_acq_rel);
    if (GPR_UNLIKELY(prev_ref_pair == MakeRefPair(0, 1))) {
      DeleteThis();
    }
  }

  // Activity methods
  void ForceImmediateRepoll() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) override;
  Waker MakeOwningWaker() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) override {
    InternalRef("wakeup");
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define GRPC_CALL_USES_ASAN_WAKER
    class AsanWaker final : public Wakeable {
     public:
      explicit AsanWaker(PromiseBasedCall* call) : call_(call) {}

      void Wakeup() override {
        call_->Wakeup();
        delete this;
      }

      void Drop() override {
        call_->Drop();
        delete this;
      }

      std::string ActivityDebugTag() const override {
        return call_->DebugTag();
      }

     private:
      PromiseBasedCall* call_;
    };
    return Waker(new AsanWaker(this));
#endif
#endif
#ifndef GRPC_CALL_USES_ASAN_WAKER
    return Waker(this);
#endif
  }
  Waker MakeNonOwningWaker() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) override;

  // Wakeable methods
  void Wakeup() override {
    grpc_event_engine::experimental::GetDefaultEventEngine()->Run([this] {
      ApplicationCallbackExecCtx app_exec_ctx;
      ExecCtx exec_ctx;
      {
        ScopedContext activity_context(this);
        MutexLock lock(&mu_);
        Update();
      }
      InternalUnref("wakeup");
    });
  }
  void Drop() override { InternalUnref("wakeup"); }

  void InContext(absl::AnyInvocable<void()> fn) {
    if (Activity::current() == this) {
      fn();
    } else {
      InternalRef("in_context");
      grpc_event_engine::experimental::GetDefaultEventEngine()->Run(
          [this, fn = std::move(fn)]() mutable {
            ExecCtx exec_ctx;
            ScopedContext activity_context(this);
            MutexLock lock(&mu_);
            fn();
            Update();
            InternalUnref("in_context");
          });
    }
  }

  grpc_compression_algorithm test_only_compression_algorithm() override {
    abort();
  }
  uint32_t test_only_message_flags() override { abort(); }
  uint32_t test_only_encodings_accepted_by_peer() override { abort(); }
  grpc_compression_algorithm compression_for_level(
      grpc_compression_level level) override {
    abort();
  }

  // This should return nullptr for the promise stack (and alternative means
  // for that functionality be invented)
  grpc_call_stack* call_stack() override { return nullptr; }

 protected:
  class ScopedContext
      : public ScopedActivity,
        public promise_detail::Context<Arena>,
        public promise_detail::Context<grpc_call_context_element>,
        public promise_detail::Context<CallContext>,
        public promise_detail::Context<CallFinalization>,
        public promise_detail::Context<FragmentAllocator> {
   public:
    explicit ScopedContext(PromiseBasedCall* call)
        : ScopedActivity(call),
          promise_detail::Context<Arena>(call->arena()),
          promise_detail::Context<grpc_call_context_element>(call->context_),
          promise_detail::Context<CallContext>(&call->call_context_),
          promise_detail::Context<CallFinalization>(&call->finalization_),
          promise_detail::Context<FragmentAllocator>(
              &call->fragment_allocator_) {}
  };

  class Completion {
   public:
    Completion() : index_(kNullIndex) {}
    ~Completion() { GPR_ASSERT(index_ == kNullIndex); }
    explicit Completion(uint8_t index) : index_(index) {}
    Completion(const Completion& other) = delete;
    Completion& operator=(const Completion& other) = delete;
    Completion(Completion&& other) noexcept : index_(other.index_) {
      other.index_ = kNullIndex;
    }
    Completion& operator=(Completion&& other) noexcept {
      GPR_ASSERT(index_ == kNullIndex);
      index_ = other.index_;
      other.index_ = kNullIndex;
      return *this;
    }

    uint8_t index() const { return index_; }
    uint8_t TakeIndex() { return absl::exchange(index_, kNullIndex); }
    bool has_value() const { return index_ != kNullIndex; }

    std::string ToString() const {
      return index_ == kNullIndex ? "null"
                                  : std::to_string(static_cast<int>(index_));
    }

   private:
    enum : uint8_t { kNullIndex = 0xff };
    uint8_t index_;
  };

  ~PromiseBasedCall() override {
    if (non_owning_wakeable_) non_owning_wakeable_->DropActivity();
  }

  enum class PauseReason {
    kStartingBatch = 0,
    kReceiveInitialMetadata,
    kReceiveStatusOnClient,
    kSendMessage,
    kReceiveMessage,
  };

  constexpr const char* PauseReasonString(PauseReason reason) {
    switch (reason) {
      case PauseReason::kStartingBatch:
        return "StartingBatch";
      case PauseReason::kReceiveInitialMetadata:
        return "ReceiveInitialMetadata";
      case PauseReason::kReceiveStatusOnClient:
        return "ReceiveStatusOnClient";
      case PauseReason::kSendMessage:
        return "SendMessage";
      case PauseReason::kReceiveMessage:
        return "ReceiveMessage";
    }
    return "Unknown";
  }

  static constexpr uint8_t PauseReasonBit(PauseReason reason) {
    return 1 << static_cast<int>(reason);
  }

  Mutex* mu() const ABSL_LOCK_RETURNED(mu_) { return &mu_; }

  Completion StartCompletion(void* tag, bool is_closure, const grpc_op* ops,
                             size_t num_ops) ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);
  void FinishCompletion(Completion* completion, PauseReason reason)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);
  void FailCompletion(const Completion& completion);
  Completion PauseCompletion(const Completion& completion, PauseReason reason)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);
  void Update() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);
  virtual void UpdateOnce() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) = 0;

  grpc_completion_queue* cq() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) { return cq_; }

  void CToMetadata(grpc_metadata* metadata, size_t count,
                   grpc_metadata_batch* batch);

  std::string ActivityDebugTag() const override { return DebugTag(); }

  void RunFinalization(grpc_status_code status, const char* status_details) {
    grpc_call_final_info final_info;
    final_info.stats = *call_context_.call_stats();
    final_info.error_string = status_details;
    finalization_.Run(&final_info);
  }

 private:
  union CompletionInfo {
    struct Pending {
      uint8_t pause_bits;
      bool is_closure;
      bool success;
      void* tag;
    } pending;
    grpc_cq_completion completion;
  };

  class NonOwningWakable final : public Wakeable {
   public:
    explicit NonOwningWakable(PromiseBasedCall* call) : call_(call) {}

    // Ref the Handle (not the activity).
    void Ref() { refs_.fetch_add(1, std::memory_order_relaxed); }

    // Activity is going away... drop its reference and sever the connection
    // back.
    void DropActivity() ABSL_LOCKS_EXCLUDED(mu_) {
      mu_.Lock();
      GPR_ASSERT(call_ != nullptr);
      call_ = nullptr;
      mu_.Unlock();
      Unref();
    }

    // Activity needs to wake up (if it still exists!) - wake it up, and drop
    // the ref that was kept for this handle.
    void Wakeup() override ABSL_LOCKS_EXCLUDED(mu_) {
      mu_.Lock();
      // Note that activity refcount can drop to zero, but we could win the lock
      // against DropActivity, so we need to only increase activities refcount
      // if it is non-zero.
      if (call_ && call_->RefIfNonZero()) {
        PromiseBasedCall* call = call_;
        mu_.Unlock();
        // Activity still exists and we have a reference: wake it up, which will
        // drop the ref.
        call->Wakeup();
      } else {
        // Could not get the activity - it's either gone or going. No need to
        // wake it up!
        mu_.Unlock();
      }
      // Drop the ref to the handle (we have one ref = one wakeup semantics).
      Unref();
    }

    std::string ActivityDebugTag() const override {
      MutexLock lock(&mu_);
      return call_ == nullptr ? "<unknown>" : call_->DebugTag();
    }

    void Drop() override { Unref(); }

   private:
    // Unref the Handle (not the activity).
    void Unref() {
      if (1 == refs_.fetch_sub(1, std::memory_order_acq_rel)) {
        delete this;
      }
    }

    mutable Mutex mu_;
    std::atomic<size_t> refs_{2};
    PromiseBasedCall* call_ ABSL_GUARDED_BY(mu_);
  };

  static void OnDestroy(void* arg, grpc_error_handle error) {
    auto* call = static_cast<PromiseBasedCall*>(arg);
    ScopedContext context(call);
    call->DeleteThis();
  }

  // First 32 bits are strong refs, next 32 bits are weak refs.
  static uint64_t MakeRefPair(uint32_t strong, uint32_t weak) {
    return (static_cast<uint64_t>(strong) << 32) + static_cast<int64_t>(weak);
  }
  static uint32_t GetStrongRefs(uint64_t ref_pair) {
    return static_cast<uint32_t>(ref_pair >> 32);
  }
  static uint32_t GetWeakRefs(uint64_t ref_pair) {
    return static_cast<uint32_t>(ref_pair & 0xffffffffu);
  }

  bool RefIfNonZero() {
    uint64_t prev_ref_pair = refs_.load(std::memory_order_acquire);
    do {
      const uint32_t strong_refs = GetStrongRefs(prev_ref_pair);
      if (strong_refs == 0) return false;
    } while (!refs_.compare_exchange_weak(
        prev_ref_pair, prev_ref_pair + MakeRefPair(1, 0),
        std::memory_order_acq_rel, std::memory_order_acquire));
    return true;
  }

  mutable Mutex mu_;
  std::atomic<uint64_t> refs_{MakeRefPair(1, 0)};
  CallContext call_context_{this};
  bool keep_polling_ ABSL_GUARDED_BY(mu()) = false;

  /* Contexts for various subsystems (security, tracing, ...). */
  grpc_call_context_element context_[GRPC_CONTEXT_COUNT] = {};
  grpc_completion_queue* cq_ ABSL_GUARDED_BY(mu_);
  FragmentAllocator fragment_allocator_ ABSL_GUARDED_BY(mu_);
  NonOwningWakable* non_owning_wakeable_ ABSL_GUARDED_BY(mu_) = nullptr;
  CompletionInfo completion_info_[6];
  CallFinalization finalization_;
};

template <typename T>
grpc_error_handle MakePromiseBasedCall(grpc_call_create_args* args,
                                       grpc_call** out_call) {
  Channel* channel = args->channel.get();

  Arena* arena;
  PromiseBasedCall* call;
  std::pair<Arena*, void*> arena_with_call = Arena::CreateWithAlloc(
      channel->CallSizeEstimate(), sizeof(T), channel->allocator());
  arena = arena_with_call.first;
  call = new (arena_with_call.second) T(arena, args);
  *out_call = call->c_ptr();
  GPR_DEBUG_ASSERT(Call::FromC(*out_call) == call);
  return GRPC_ERROR_NONE;
}

PromiseBasedCall::PromiseBasedCall(Arena* arena,
                                   const grpc_call_create_args& args)
    : Call(arena, args.server_transport_data == nullptr, args.send_deadline,
           args.channel->Ref()),
      cq_(args.cq) {}

Waker PromiseBasedCall::MakeNonOwningWaker() {
  if (non_owning_wakeable_ == nullptr) {
    non_owning_wakeable_ = new NonOwningWakable(this);
  } else {
    non_owning_wakeable_->Ref();
  }
  return Waker(non_owning_wakeable_);
}

void PromiseBasedCall::CToMetadata(grpc_metadata* metadata, size_t count,
                                   grpc_metadata_batch* b) {
  // DO NOT SUBMIT
  // Need to add a ValidateMetadata to the op validation loop.
  for (size_t i = 0; i < count; i++) {
    grpc_metadata* md = &metadata[i];
    auto key = StringViewFromSlice(md->key);
    // Filter "content-length metadata"
    if (key == "content-length") continue;
    b->Append(key, Slice(grpc_slice_ref_internal(md->value)),
              [md](absl::string_view error, const Slice& value) {
                gpr_log(GPR_DEBUG, "Append error: %s",
                        absl::StrCat("key=", StringViewFromSlice(md->key),
                                     " error=", error,
                                     " value=", value.as_string_view())
                            .c_str());
              });
  }
}

void PromiseBasedCall::ContextSet(grpc_context_index elem, void* value,
                                  void (*destroy)(void*)) {
  if (context_[elem].destroy) {
    context_[elem].destroy(context_[elem].value);
  }
  context_[elem].value = value;
  context_[elem].destroy = destroy;
}

void* PromiseBasedCall::ContextGet(grpc_context_index elem) const {
  return context_[elem].value;
}

PromiseBasedCall::Completion PromiseBasedCall::StartCompletion(
    void* tag, bool is_closure, const grpc_op* ops, size_t num_ops) {
  Completion c(BatchSlotForOp(ops[0].op));
  if (grpc_call_trace.enabled()) {
    gpr_log(GPR_INFO, "%sStartCompletion %s tag=%p", DebugTag().c_str(),
            c.ToString().c_str(), tag);
  }
  grpc_cq_begin_op(cq(), tag);
  completion_info_[c.index()].pending = {
      PauseReasonBit(PauseReason::kStartingBatch), is_closure, true, tag};
  return c;
}

PromiseBasedCall::Completion PromiseBasedCall::PauseCompletion(
    const Completion& completion, PauseReason reason) {
  if (grpc_call_trace.enabled()) {
    gpr_log(GPR_INFO, "%sPauseCompletion %s %s", DebugTag().c_str(),
            completion.ToString().c_str(), PauseReasonString(reason));
  }
  auto& pause_bits = completion_info_[completion.index()].pending.pause_bits;
  GPR_ASSERT((pause_bits & PauseReasonBit(reason)) == 0);
  pause_bits |= PauseReasonBit(reason);
  return Completion(completion.index());
}

void PromiseBasedCall::FailCompletion(const Completion& completion) {
  if (grpc_call_trace.enabled()) {
    gpr_log(GPR_INFO, "%sFailCompletion %s", DebugTag().c_str(),
            completion.ToString().c_str());
  }
  completion_info_[completion.index()].pending.success = false;
}

void PromiseBasedCall::FinishCompletion(Completion* completion,
                                        PauseReason reason) {
  if (grpc_call_trace.enabled()) {
    auto pause_bits = completion_info_[completion->index()].pending.pause_bits;
    bool success = completion_info_[completion->index()].pending.success;
    std::vector<const char*> pending;
    for (size_t i = 0; i < 8 * sizeof(pause_bits); i++) {
      if (static_cast<PauseReason>(i) == reason) continue;
      if (pause_bits & (1 << i)) {
        pending.push_back(PauseReasonString(static_cast<PauseReason>(i)));
      }
    }
    gpr_log(GPR_INFO, "%sFinishCompletion %s %s %s", DebugTag().c_str(),
            completion->ToString().c_str(), PauseReasonString(reason),
            (pending.empty()
                 ? (success ? std::string("done") : std::string("failed"))
                 : absl::StrFormat("paused:remaining={%s}",
                                   absl::StrJoin(pending, ",")))
                .c_str());
  }
  const uint8_t i = completion->TakeIndex();
  GPR_ASSERT(i < GPR_ARRAY_SIZE(completion_info_));
  CompletionInfo::Pending& pending = completion_info_[i].pending;
  GPR_ASSERT(pending.pause_bits & PauseReasonBit(reason));
  pending.pause_bits &= ~PauseReasonBit(reason);
  auto error = pending.success ? GRPC_ERROR_NONE : GRPC_ERROR_CANCELLED;
  if (pending.pause_bits == 0) {
    if (pending.is_closure) {
      ExecCtx::Run(DEBUG_LOCATION, static_cast<grpc_closure*>(pending.tag),
                   error);
    } else {
      grpc_cq_end_op(
          cq(), pending.tag, error, [](void*, grpc_cq_completion*) {}, nullptr,
          &completion_info_[i].completion);
    }
  }
}

void PromiseBasedCall::Update() {
  keep_polling_ = false;
  do {
    UpdateOnce();
  } while (absl::exchange(keep_polling_, false));
}

void PromiseBasedCall::ForceImmediateRepoll() { keep_polling_ = true; }

void PromiseBasedCall::SetCompletionQueue(grpc_completion_queue* cq) {
  MutexLock lock(&mu_);
  cq_ = cq;
}

///////////////////////////////////////////////////////////////////////////////
// CallContext

void CallContext::InContext(absl::AnyInvocable<void()> fn) {
  call_->InContext(std::move(fn));
}

void CallContext::IncrementRefCount(const char* reason) {
  call_->InternalRef(reason);
}

void CallContext::Unref(const char* reason) { call_->InternalUnref(reason); }

///////////////////////////////////////////////////////////////////////////////
// ClientPromiseBasedCall

class ClientPromiseBasedCall final : public PromiseBasedCall {
 public:
  ClientPromiseBasedCall(Arena* arena, grpc_call_create_args* args)
      : PromiseBasedCall(arena, *args) {
    GRPC_STATS_INC_CLIENT_CALLS_CREATED();
    ScopedContext context(this);
    send_initial_metadata_ =
        GetContext<FragmentAllocator>()->MakeClientMetadata();
    send_initial_metadata_->Set(HttpPathMetadata(), std::move(*args->path));
    if (args->authority.has_value()) {
      send_initial_metadata_->Set(HttpAuthorityMetadata(),
                                  std::move(*args->authority));
    }
    if (auto* channelz_channel = channel()->channelz_node()) {
      channelz_channel->RecordCallStarted();
    }
  }

  ~ClientPromiseBasedCall() override {
    ScopedContext context(this);
    send_initial_metadata_.reset();
    recv_status_on_client_ = absl::monostate();
    promise_ = ArenaPromise<ServerMetadataHandle>();
    auto c2s = std::move(client_to_server_messages_);
    auto s2c = std::move(server_to_client_messages_);
  }

  absl::string_view GetServerAuthority() const override { abort(); }
  void CancelWithError(grpc_error_handle error) override;
  bool Completed() override;
  void Orphan() override {
    MutexLock lock(mu());
    ScopedContext ctx(this);
    if (!completed_) Finish(ServerMetadataHandle(absl::CancelledError()));
  }
  bool is_trailers_only() const override {
    MutexLock lock(mu());
    return is_trailers_only_;
  }
  bool failed_before_recv_message() const override { abort(); }

  grpc_call_error StartBatch(const grpc_op* ops, size_t nops, void* notify_tag,
                             bool is_notify_tag_closure) override;

  std::string DebugTag() const override {
    return absl::StrFormat("CLIENT_CALL[%p]: ", this);
  }

 private:
  void UpdateOnce() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu()) override;
  void Finish(ServerMetadataHandle trailing_metadata)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu());
  grpc_call_error ValidateBatch(const grpc_op* ops, size_t nops)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu());
  void CommitBatch(const grpc_op* ops, size_t nops,
                   const Completion& completion)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu());
  void StartPromise(ClientMetadataHandle client_initial_metadata)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu());
  void MaybePublishResult() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu());
  static void PublishMetadataArray(grpc_metadata_array* array,
                                   ServerMetadata* md);
  void PublishStatus(
      grpc_op::grpc_op_data::grpc_op_recv_status_on_client op_args,
      ServerMetadataHandle trailing_metadata)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu());
  void PublishInitialMetadata(ServerMetadata* metadata)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu());

  ArenaPromise<ServerMetadataHandle> promise_ ABSL_GUARDED_BY(mu());
  Latch<ServerMetadata*> server_initial_metadata_ ABSL_GUARDED_BY(mu());
  Pipe<MessageHandle> client_to_server_messages_ ABSL_GUARDED_BY(mu()){arena()};
  Pipe<MessageHandle> server_to_client_messages_ ABSL_GUARDED_BY(mu()){arena()};

  ClientMetadataHandle send_initial_metadata_;
  grpc_metadata_array* recv_initial_metadata_ ABSL_GUARDED_BY(mu()) = nullptr;
  grpc_byte_buffer** recv_message_ ABSL_GUARDED_BY(mu()) = nullptr;
  AtomicWaker send_message_waker_;
  absl::variant<absl::monostate,
                grpc_op::grpc_op_data::grpc_op_recv_status_on_client,
                ServerMetadataHandle>
      recv_status_on_client_ ABSL_GUARDED_BY(mu());
  absl::optional<PipeSender<MessageHandle>::PushType> outstanding_send_
      ABSL_GUARDED_BY(mu());
  absl::optional<PipeReceiver<MessageHandle>::NextType> outstanding_recv_
      ABSL_GUARDED_BY(mu());
  absl::optional<Latch<ServerMetadata*>::WaitPromise>
      server_initial_metadata_ready_;
  absl::optional<grpc_compression_algorithm> incoming_compression_algorithm_;
  Completion recv_initial_metadata_completion_ ABSL_GUARDED_BY(mu());
  Completion recv_status_on_client_completion_ ABSL_GUARDED_BY(mu());
  Completion send_message_completion_ ABSL_GUARDED_BY(mu());
  Completion recv_message_completion_ ABSL_GUARDED_BY(mu());
  bool completed_ ABSL_GUARDED_BY(mu()) = false;
  bool is_trailers_only_ ABSL_GUARDED_BY(mu());
};

void ClientPromiseBasedCall::StartPromise(
    ClientMetadataHandle client_initial_metadata) {
  GPR_ASSERT(!promise_.has_value());
  promise_ = channel()->channel_stack()->MakeClientCallPromise(CallArgs{
      std::move(client_initial_metadata),
      &server_initial_metadata_,
      &client_to_server_messages_.receiver,
      &server_to_client_messages_.sender,
  });
}

void ClientPromiseBasedCall::CancelWithError(grpc_error_handle error) {
  MutexLock lock(mu());
  ScopedContext context(this);
  Finish(ServerMetadataHandle(grpc_error_to_absl_status(error)));
  GRPC_ERROR_UNREF(error);
}

grpc_call_error ClientPromiseBasedCall::ValidateBatch(const grpc_op* ops,
                                                      size_t nops) {
  BitSet<8> got_ops;
  for (size_t op_idx = 0; op_idx < nops; op_idx++) {
    const grpc_op& op = ops[op_idx];
    switch (op.op) {
      case GRPC_OP_SEND_INITIAL_METADATA:
        if (!AreInitialMetadataFlagsValid(op.flags)) {
          return GRPC_CALL_ERROR_INVALID_FLAGS;
        }
        break;
      case GRPC_OP_SEND_MESSAGE:
        if (!AreWriteFlagsValid(op.flags)) {
          return GRPC_CALL_ERROR_INVALID_FLAGS;
        }
        break;
      case GRPC_OP_RECV_INITIAL_METADATA:
      case GRPC_OP_RECV_MESSAGE:
      case GRPC_OP_SEND_CLOSE_FROM_CLIENT:
      case GRPC_OP_RECV_STATUS_ON_CLIENT:
        if (op.flags != 0) return GRPC_CALL_ERROR_INVALID_FLAGS;
        break;
      case GRPC_OP_RECV_CLOSE_ON_SERVER:
      case GRPC_OP_SEND_STATUS_FROM_SERVER:
        return GRPC_CALL_ERROR_NOT_ON_CLIENT;
    }
    if (got_ops.is_set(op.op)) return GRPC_CALL_ERROR_TOO_MANY_OPERATIONS;
    got_ops.set(op.op);
  }
  return GRPC_CALL_OK;
}

void ClientPromiseBasedCall::CommitBatch(const grpc_op* ops, size_t nops,
                                         const Completion& completion) {
  for (size_t op_idx = 0; op_idx < nops; op_idx++) {
    const grpc_op& op = ops[op_idx];
    switch (op.op) {
      case GRPC_OP_SEND_INITIAL_METADATA: {
        // compression not implemented
        GPR_ASSERT(
            !op.data.send_initial_metadata.maybe_compression_level.is_set);
        if (!completed_) {
          CToMetadata(op.data.send_initial_metadata.metadata,
                      op.data.send_initial_metadata.count,
                      send_initial_metadata_.get());
          StartPromise(std::move(send_initial_metadata_));
        }
      } break;
      case GRPC_OP_RECV_INITIAL_METADATA: {
        recv_initial_metadata_ =
            op.data.recv_initial_metadata.recv_initial_metadata;
        server_initial_metadata_ready_ = server_initial_metadata_.Wait();
        recv_initial_metadata_completion_ =
            PauseCompletion(completion, PauseReason::kReceiveInitialMetadata);
      } break;
      case GRPC_OP_RECV_STATUS_ON_CLIENT: {
        recv_status_on_client_completion_ =
            PauseCompletion(completion, PauseReason::kReceiveStatusOnClient);
        if (auto* finished_metadata =
                absl::get_if<ServerMetadataHandle>(&recv_status_on_client_)) {
          PublishStatus(op.data.recv_status_on_client,
                        std::move(*finished_metadata));
        } else {
          recv_status_on_client_ = op.data.recv_status_on_client;
        }
      } break;
      case GRPC_OP_SEND_MESSAGE: {
        GPR_ASSERT(!outstanding_send_.has_value());
        if (!completed_) {
          send_message_completion_ =
              PauseCompletion(completion, PauseReason::kSendMessage);
          SliceBuffer send;
          grpc_slice_buffer_swap(
              &op.data.send_message.send_message->data.raw.slice_buffer,
              send.c_slice_buffer());
          outstanding_send_.emplace(client_to_server_messages_.sender.Push(
              GetContext<FragmentAllocator>()->MakeMessage(
                  std::move(send), op.flags,
                  [this]() { send_message_waker_.Wakeup(); })));
        } else {
          FailCompletion(completion);
        }
      } break;
      case GRPC_OP_RECV_MESSAGE: {
        GPR_ASSERT(!outstanding_recv_.has_value());
        recv_message_ = op.data.recv_message.recv_message;
        recv_message_completion_ =
            PauseCompletion(completion, PauseReason::kReceiveMessage);
        outstanding_recv_.emplace(server_to_client_messages_.receiver.Next());
      } break;
      case GRPC_OP_SEND_CLOSE_FROM_CLIENT: {
        client_to_server_messages_.sender.Close();
      } break;
      case GRPC_OP_SEND_STATUS_FROM_SERVER:
      case GRPC_OP_RECV_CLOSE_ON_SERVER:
        abort();  // unreachable
    }
  }
}

grpc_call_error ClientPromiseBasedCall::StartBatch(const grpc_op* ops,
                                                   size_t nops,
                                                   void* notify_tag,
                                                   bool is_notify_tag_closure) {
  MutexLock lock(mu());
  ScopedContext activity_context(this);
  if (nops == 0) {
    EndOpImmediately(cq(), notify_tag, is_notify_tag_closure);
    return GRPC_CALL_OK;
  }
  const grpc_call_error validation_result = ValidateBatch(ops, nops);
  if (validation_result != GRPC_CALL_OK) {
    return validation_result;
  }
  Completion completion =
      StartCompletion(notify_tag, is_notify_tag_closure, ops, nops);
  CommitBatch(ops, nops, completion);
  Update();
  FinishCompletion(&completion, PauseReason::kStartingBatch);
  return GRPC_CALL_OK;
}

void ClientPromiseBasedCall::PublishInitialMetadata(ServerMetadata* metadata) {
  incoming_compression_algorithm_ =
      metadata->Take(GrpcEncodingMetadata()).value_or(GRPC_COMPRESS_NONE);
  server_initial_metadata_ready_.reset();
  GPR_ASSERT(recv_initial_metadata_ != nullptr);
  PublishMetadataArray(absl::exchange(recv_initial_metadata_, nullptr),
                       metadata);
  FinishCompletion(&recv_initial_metadata_completion_,
                   PauseReason::kReceiveInitialMetadata);
}

void ClientPromiseBasedCall::UpdateOnce() {
  if (grpc_call_trace.enabled()) {
    auto present_and_completion_text =
        [](const char* caption, bool has,
           const Completion& completion) -> std::string {
      if (has) {
        if (completion.has_value()) {
          return absl::StrCat(caption, ":",
                              static_cast<int>(completion.index()), " ");
        } else {
          return absl::StrCat(caption,
                              ":!!BUG:operation is present, no completion!! ");
        }
      } else {
        if (!completion.has_value()) {
          return "";
        } else {
          return absl::StrCat(
              caption, ":!!BUG:operation is not present, has completion: ",
              static_cast<int>(completion.index()), "!! ");
        }
      }
    };
    gpr_log(
        GPR_INFO, "%sUpdateOnce: %s%s%shas_promise=%s", DebugTag().c_str(),
        present_and_completion_text("server_initial_metadata_ready",
                                    server_initial_metadata_ready_.has_value(),
                                    recv_initial_metadata_completion_)
            .c_str(),
        present_and_completion_text("outstanding_send",
                                    outstanding_send_.has_value(),
                                    send_message_completion_)
            .c_str(),
        present_and_completion_text("outstanding_recv",
                                    outstanding_recv_.has_value(),
                                    recv_message_completion_)
            .c_str(),
        promise_.has_value() ? "true" : "false");
  }
  if (send_message_completion_.has_value() && !send_message_waker_.Armed()) {
    FinishCompletion(&send_message_completion_, PauseReason::kSendMessage);
  }
  if (server_initial_metadata_ready_.has_value()) {
    Poll<ServerMetadata**> r = (*server_initial_metadata_ready_)();
    if (ServerMetadata*** server_initial_metadata =
            absl::get_if<ServerMetadata**>(&r)) {
      PublishInitialMetadata(**server_initial_metadata);
    } else if (completed_) {
      ServerMetadata no_metadata{GetContext<Arena>()};
      PublishInitialMetadata(&no_metadata);
    }
  }
  if (outstanding_send_.has_value()) {
    Poll<bool> r = (*outstanding_send_)();
    if (const bool* result = absl::get_if<bool>(&r)) {
      outstanding_send_.reset();
      if (!*result) {
        FailCompletion(send_message_completion_);
        Finish(ServerMetadataHandle(absl::Status(
            absl::StatusCode::kInternal, "Failed to send message to server")));
      }
    }
  }
  if (promise_.has_value()) {
    Poll<ServerMetadataHandle> r = promise_();
    if (grpc_call_trace.enabled()) {
      gpr_log(GPR_INFO, "%sUpdateOnce: promise returns %s", DebugTag().c_str(),
              PollToString(r, [](const ServerMetadataHandle& h) {
                return h->DebugString();
              }).c_str());
    }
    if (auto* result = absl::get_if<ServerMetadataHandle>(&r)) {
      Finish(std::move(*result));
    }
  }
  if (incoming_compression_algorithm_.has_value() &&
      outstanding_recv_.has_value()) {
    Poll<absl::optional<MessageHandle>> r = (*outstanding_recv_)();
    if (auto* result = absl::get_if<absl::optional<MessageHandle>>(&r)) {
      outstanding_recv_.reset();
      if (result->has_value()) {
        MessageHandle& message = **result;
        if ((message->flags() & GRPC_WRITE_INTERNAL_COMPRESS) &&
            (incoming_compression_algorithm_ != GRPC_COMPRESS_NONE)) {
          *recv_message_ = grpc_raw_compressed_byte_buffer_create(
              nullptr, 0, *incoming_compression_algorithm_);
        } else {
          *recv_message_ = grpc_raw_byte_buffer_create(nullptr, 0);
        }
        grpc_slice_buffer_move_into(message->payload()->c_slice_buffer(),
                                    &(*recv_message_)->data.raw.slice_buffer);
        if (grpc_call_trace.enabled()) {
          gpr_log(GPR_INFO,
                  "%sUpdateOnce: outstanding_recv finishes: received %" PRIdPTR
                  " byte message",
                  DebugTag().c_str(),
                  (*recv_message_)->data.raw.slice_buffer.length);
        }
      } else {
        if (grpc_call_trace.enabled()) {
          gpr_log(
              GPR_INFO,
              "%sUpdateOnce: outstanding_recv finishes: received end-of-stream",
              DebugTag().c_str());
        }
        *recv_message_ = nullptr;
      }
      FinishCompletion(&recv_message_completion_, PauseReason::kReceiveMessage);
    } else if (completed_) {
      if (grpc_call_trace.enabled()) {
        gpr_log(GPR_INFO,
                "%sUpdateOnce: outstanding_recv finishes: promise has "
                "completed without queuing a message, forcing end-of-stream",
                DebugTag().c_str());
      }
      outstanding_recv_.reset();
      *recv_message_ = nullptr;
      FinishCompletion(&recv_message_completion_, PauseReason::kReceiveMessage);
    }
  }
}

void ClientPromiseBasedCall::Finish(ServerMetadataHandle trailing_metadata) {
  if (grpc_call_trace.enabled()) {
    gpr_log(GPR_INFO, "%sFinish: %s", DebugTag().c_str(),
            trailing_metadata->DebugString().c_str());
  }
  promise_ = ArenaPromise<ServerMetadataHandle>();
  completed_ = true;
  if (recv_initial_metadata_ != nullptr) {
    ForceImmediateRepoll();
  }
  const bool pending_initial_metadata =
      server_initial_metadata_ready_.has_value();
  server_initial_metadata_ready_.reset();
  Poll<ServerMetadata**> r = server_initial_metadata_.Wait()();
  if (auto* result = absl::get_if<ServerMetadata**>(&r)) {
    if (pending_initial_metadata) PublishInitialMetadata(**result);
    is_trailers_only_ = false;
  } else {
    if (pending_initial_metadata) {
      ServerMetadata no_metadata{GetContext<Arena>()};
      PublishInitialMetadata(&no_metadata);
    }
    is_trailers_only_ = true;
  }
  if (auto* channelz_channel = channel()->channelz_node()) {
    if (trailing_metadata->get(GrpcStatusMetadata())
            .value_or(GRPC_STATUS_UNKNOWN) == GRPC_STATUS_OK) {
      channelz_channel->RecordCallSucceeded();
    } else {
      channelz_channel->RecordCallFailed();
    }
  }
  if (auto* status_request =
          absl::get_if<grpc_op::grpc_op_data::grpc_op_recv_status_on_client>(
              &recv_status_on_client_)) {
    PublishStatus(*status_request, std::move(trailing_metadata));
  } else {
    recv_status_on_client_ = std::move(trailing_metadata);
  }
}

namespace {
std::string MakeErrorString(const ServerMetadata* trailing_metadata) {
  std::string out = absl::StrCat(
      trailing_metadata->get(GrpcStatusFromWire()).value_or(false)
          ? "Error received from peer"
          : "Error generated by client",
      "grpc_status: ",
      grpc_status_code_to_string(trailing_metadata->get(GrpcStatusMetadata())
                                     .value_or(GRPC_STATUS_UNKNOWN)));
  if (const Slice* message =
          trailing_metadata->get_pointer(GrpcMessageMetadata())) {
    absl::StrAppend(&out, "\ngrpc_message: ", message->as_string_view());
  }
  if (auto annotations = trailing_metadata->get_pointer(GrpcStatusContext())) {
    absl::StrAppend(&out, "\nStatus Context:");
    for (const std::string& annotation : *annotations) {
      absl::StrAppend(&out, "\n  ", annotation);
    }
  }
  return out;
}
}  // namespace

void ClientPromiseBasedCall::PublishStatus(
    grpc_op::grpc_op_data::grpc_op_recv_status_on_client op_args,
    ServerMetadataHandle trailing_metadata) {
  const grpc_status_code status = trailing_metadata->get(GrpcStatusMetadata())
                                      .value_or(GRPC_STATUS_UNKNOWN);
  *op_args.status = status;
  absl::string_view message_string;
  if (Slice* message = trailing_metadata->get_pointer(GrpcMessageMetadata())) {
    message_string = message->as_string_view();
    *op_args.status_details = message->Ref().TakeCSlice();
  } else {
    *op_args.status_details = grpc_empty_slice();
  }
  if (message_string.empty()) {
    RunFinalization(status, nullptr);
  } else {
    std::string error_string(message_string);
    RunFinalization(status, error_string.c_str());
  }
  if (op_args.error_string != nullptr && status != GRPC_STATUS_OK) {
    *op_args.error_string =
        gpr_strdup(MakeErrorString(trailing_metadata.get()).c_str());
  }
  PublishMetadataArray(op_args.trailing_metadata, trailing_metadata.get());
  FinishCompletion(&recv_status_on_client_completion_,
                   PauseReason::kReceiveStatusOnClient);
}

void ClientPromiseBasedCall::PublishMetadataArray(grpc_metadata_array* array,
                                                  ServerMetadata* md) {
  const auto md_count = md->count();
  if (md_count > array->capacity) {
    array->capacity =
        std::max(array->capacity + md->count(), array->capacity * 3 / 2);
    array->metadata = static_cast<grpc_metadata*>(
        gpr_realloc(array->metadata, sizeof(grpc_metadata) * array->capacity));
  }
  PublishToAppEncoder encoder(array);
  md->Encode(&encoder);
}

bool ClientPromiseBasedCall::Completed() {
  MutexLock lock(mu());
  return completed_;
}

gpr_atm* CallContext::peer_string_atm_ptr() {
  return call_->peer_string_atm_ptr();
}

}  // namespace grpc_core

///////////////////////////////////////////////////////////////////////////////
// C-based API

void* grpc_call_arena_alloc(grpc_call* call, size_t size) {
  grpc_core::ExecCtx exec_ctx;
  return grpc_core::Call::FromC(call)->arena()->Alloc(size);
}

size_t grpc_call_get_initial_size_estimate() {
  return grpc_core::FilterStackCall::InitialSizeEstimate();
}

grpc_error_handle grpc_call_create(grpc_call_create_args* args,
                                   grpc_call** out_call) {
  static const bool kAllowPromises =
      GPR_GLOBAL_CONFIG_GET(grpc_experimental_enable_promise_based_call);
  if (kAllowPromises && args->channel->is_promising()) {
    if (args->server_transport_data == nullptr) {
      return grpc_core::MakePromiseBasedCall<grpc_core::ClientPromiseBasedCall>(
          args, out_call);
    }
  }
  return grpc_core::FilterStackCall::Create(args, out_call);
}

void grpc_call_set_completion_queue(grpc_call* call,
                                    grpc_completion_queue* cq) {
  grpc_core::Call::FromC(call)->SetCompletionQueue(cq);
}

void grpc_call_ref(grpc_call* c) { grpc_core::Call::FromC(c)->ExternalRef(); }

void grpc_call_unref(grpc_call* c) {
  grpc_core::ExecCtx exec_ctx;
  grpc_core::Call::FromC(c)->ExternalUnref();
}

char* grpc_call_get_peer(grpc_call* call) {
  return grpc_core::Call::FromC(call)->GetPeer();
}

grpc_call* grpc_call_from_top_element(grpc_call_element* surface_element) {
  return grpc_core::FilterStackCall::FromTopElem(surface_element)->c_ptr();
}

grpc_call_error grpc_call_cancel(grpc_call* call, void* reserved) {
  GRPC_API_TRACE("grpc_call_cancel(call=%p, reserved=%p)", 2, (call, reserved));
  GPR_ASSERT(reserved == nullptr);
  grpc_core::ApplicationCallbackExecCtx callback_exec_ctx;
  grpc_core::ExecCtx exec_ctx;
  grpc_core::Call::FromC(call)->CancelWithError(GRPC_ERROR_CANCELLED);
  return GRPC_CALL_OK;
}

grpc_call_error grpc_call_cancel_with_status(grpc_call* c,
                                             grpc_status_code status,
                                             const char* description,
                                             void* reserved) {
  GRPC_API_TRACE(
      "grpc_call_cancel_with_status("
      "c=%p, status=%d, description=%s, reserved=%p)",
      4, (c, (int)status, description, reserved));
  GPR_ASSERT(reserved == nullptr);
  grpc_core::ApplicationCallbackExecCtx callback_exec_ctx;
  grpc_core::ExecCtx exec_ctx;
  grpc_core::Call::FromC(c)->CancelWithStatus(status, description);
  return GRPC_CALL_OK;
}

void grpc_call_cancel_internal(grpc_call* call) {
  grpc_core::Call::FromC(call)->CancelWithError(GRPC_ERROR_CANCELLED);
}

grpc_compression_algorithm grpc_call_test_only_get_compression_algorithm(
    grpc_call* call) {
  return grpc_core::Call::FromC(call)->test_only_compression_algorithm();
}

uint32_t grpc_call_test_only_get_message_flags(grpc_call* call) {
  return grpc_core::Call::FromC(call)->test_only_message_flags();
}

uint32_t grpc_call_test_only_get_encodings_accepted_by_peer(grpc_call* call) {
  return grpc_core::Call::FromC(call)->test_only_encodings_accepted_by_peer();
}

grpc_core::Arena* grpc_call_get_arena(grpc_call* call) {
  return grpc_core::Call::FromC(call)->arena();
}

grpc_call_stack* grpc_call_get_call_stack(grpc_call* call) {
  return grpc_core::Call::FromC(call)->call_stack();
}

grpc_call_error grpc_call_start_batch(grpc_call* call, const grpc_op* ops,
                                      size_t nops, void* tag, void* reserved) {
  GRPC_API_TRACE(
      "grpc_call_start_batch(call=%p, ops=%p, nops=%lu, tag=%p, "
      "reserved=%p)",
      5, (call, ops, (unsigned long)nops, tag, reserved));

  if (reserved != nullptr) {
    return GRPC_CALL_ERROR;
  } else {
    grpc_core::ApplicationCallbackExecCtx callback_exec_ctx;
    grpc_core::ExecCtx exec_ctx;
    return grpc_core::Call::FromC(call)->StartBatch(ops, nops, tag, false);
  }
}

grpc_call_error grpc_call_start_batch_and_execute(grpc_call* call,
                                                  const grpc_op* ops,
                                                  size_t nops,
                                                  grpc_closure* closure) {
  return grpc_core::Call::FromC(call)->StartBatch(ops, nops, closure, true);
}

void grpc_call_context_set(grpc_call* call, grpc_context_index elem,
                           void* value, void (*destroy)(void* value)) {
  return grpc_core::Call::FromC(call)->ContextSet(elem, value, destroy);
}

void* grpc_call_context_get(grpc_call* call, grpc_context_index elem) {
  return grpc_core::Call::FromC(call)->ContextGet(elem);
}

uint8_t grpc_call_is_client(grpc_call* call) {
  return grpc_core::Call::FromC(call)->is_client();
}

grpc_compression_algorithm grpc_call_compression_for_level(
    grpc_call* call, grpc_compression_level level) {
  return grpc_core::Call::FromC(call)->compression_for_level(level);
}

bool grpc_call_is_trailers_only(const grpc_call* call) {
  return grpc_core::Call::FromC(call)->is_trailers_only();
}

int grpc_call_failed_before_recv_message(const grpc_call* c) {
  return grpc_core::Call::FromC(c)->failed_before_recv_message();
}

absl::string_view grpc_call_server_authority(const grpc_call* call) {
  return grpc_core::Call::FromC(call)->GetServerAuthority();
}

const char* grpc_call_error_to_string(grpc_call_error error) {
  switch (error) {
    case GRPC_CALL_ERROR:
      return "GRPC_CALL_ERROR";
    case GRPC_CALL_ERROR_ALREADY_ACCEPTED:
      return "GRPC_CALL_ERROR_ALREADY_ACCEPTED";
    case GRPC_CALL_ERROR_ALREADY_FINISHED:
      return "GRPC_CALL_ERROR_ALREADY_FINISHED";
    case GRPC_CALL_ERROR_ALREADY_INVOKED:
      return "GRPC_CALL_ERROR_ALREADY_INVOKED";
    case GRPC_CALL_ERROR_BATCH_TOO_BIG:
      return "GRPC_CALL_ERROR_BATCH_TOO_BIG";
    case GRPC_CALL_ERROR_INVALID_FLAGS:
      return "GRPC_CALL_ERROR_INVALID_FLAGS";
    case GRPC_CALL_ERROR_INVALID_MESSAGE:
      return "GRPC_CALL_ERROR_INVALID_MESSAGE";
    case GRPC_CALL_ERROR_INVALID_METADATA:
      return "GRPC_CALL_ERROR_INVALID_METADATA";
    case GRPC_CALL_ERROR_NOT_INVOKED:
      return "GRPC_CALL_ERROR_NOT_INVOKED";
    case GRPC_CALL_ERROR_NOT_ON_CLIENT:
      return "GRPC_CALL_ERROR_NOT_ON_CLIENT";
    case GRPC_CALL_ERROR_NOT_ON_SERVER:
      return "GRPC_CALL_ERROR_NOT_ON_SERVER";
    case GRPC_CALL_ERROR_NOT_SERVER_COMPLETION_QUEUE:
      return "GRPC_CALL_ERROR_NOT_SERVER_COMPLETION_QUEUE";
    case GRPC_CALL_ERROR_PAYLOAD_TYPE_MISMATCH:
      return "GRPC_CALL_ERROR_PAYLOAD_TYPE_MISMATCH";
    case GRPC_CALL_ERROR_TOO_MANY_OPERATIONS:
      return "GRPC_CALL_ERROR_TOO_MANY_OPERATIONS";
    case GRPC_CALL_ERROR_COMPLETION_QUEUE_SHUTDOWN:
      return "GRPC_CALL_ERROR_COMPLETION_QUEUE_SHUTDOWN";
    case GRPC_CALL_OK:
      return "GRPC_CALL_OK";
  }
  GPR_UNREACHABLE_CODE(return "GRPC_CALL_ERROR_UNKNOW");
}
