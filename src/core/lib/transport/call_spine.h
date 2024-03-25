// Copyright 2024 gRPC authors.
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

#ifndef GRPC_SRC_CORE_LIB_TRANSPORT_CALL_SPINE_H
#define GRPC_SRC_CORE_LIB_TRANSPORT_CALL_SPINE_H

#include <grpc/support/port_platform.h>

#include <grpc/event_engine/event_engine.h>
#include <grpc/support/log.h>

#include "src/core/lib/channel/context.h"
#include "src/core/lib/gprpp/dual_ref_counted.h"
#include "src/core/lib/promise/detail/status.h"
#include "src/core/lib/promise/if.h"
#include "src/core/lib/promise/latch.h"
#include "src/core/lib/promise/party.h"
#include "src/core/lib/promise/status_flag.h"
#include "src/core/lib/promise/try_seq.h"
#include "src/core/lib/transport/call_filters.h"
#include "src/core/lib/transport/message.h"
#include "src/core/lib/transport/metadata.h"

namespace grpc_core {

class CallSpine final : public Party {
 public:
  static RefCountedPtr<CallSpine> Create(
      ClientMetadataHandle client_initial_metadata,
      grpc_event_engine::experimental::EventEngine* event_engine, Arena* arena,
      bool arena_is_owned, grpc_call_context_element* legacy_context) {
    return RefCountedPtr<CallSpine>(
        arena->New<CallSpine>(std::move(client_initial_metadata), event_engine,
                              arena, arena_is_owned, legacy_context));
  }

  ~CallSpine() override {
    if (legacy_context_is_owned_) {
      for (size_t i = 0; i < GRPC_CONTEXT_COUNT; i++) {
        grpc_call_context_element& elem = legacy_context_[i];
        if (elem.destroy != nullptr) elem.destroy(&elem);
      }
    }
  }

  CallFilters& call_filters() { return call_filters_; }

  // Wrap a promise so that if it returns failure it automatically cancels
  // the rest of the call.
  // The resulting (returned) promise will resolve to Empty.
  template <typename Promise>
  auto CancelIfFails(Promise promise) {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == this);
    using P = promise_detail::PromiseLike<Promise>;
    using ResultType = typename P::Result;
    return Map(std::move(promise), [this](ResultType r) {
      if (!IsStatusOk(r)) {
        call_filters_.PushServerTrailingMetadata(
            StatusCast<ServerMetadataHandle>(r));
      }
      return r;
    });
  }

  // Spawn a promise that returns Empty{} and save some boilerplate handling
  // that detail.
  template <typename PromiseFactory>
  void SpawnInfallible(absl::string_view name, PromiseFactory promise_factory) {
    Spawn(name, std::move(promise_factory), [](Empty) {});
  }

  // Spawn a promise that returns some status-like type; if the status
  // represents failure automatically cancel the rest of the call.
  template <typename PromiseFactory>
  void SpawnGuarded(absl::string_view name, PromiseFactory promise_factory) {
    using FactoryType =
        promise_detail::OncePromiseFactory<void, PromiseFactory>;
    using PromiseType = typename FactoryType::Promise;
    using ResultType = typename PromiseType::Result;
    static_assert(
        std::is_same<bool,
                     decltype(IsStatusOk(std::declval<ResultType>()))>::value,
        "SpawnGuarded promise must return a status-like object");
    Spawn(name, std::move(promise_factory), [this](ResultType r) {
      if (!IsStatusOk(r)) {
        if (grpc_trace_promise_primitives.enabled()) {
          gpr_log(GPR_DEBUG, "SpawnGuarded sees failure: %s",
                  r.ToString().c_str());
        }
        call_filters_.PushServerTrailingMetadata(
            StatusCast<ServerMetadataHandle>(std::move(r)));
      }
    });
  }

  grpc_call_context_element& legacy_context(grpc_context_index index) const {
    return legacy_context_[index];
  }

  grpc_event_engine::experimental::EventEngine* event_engine() const override {
    return event_engine_;
  }

  Arena* arena() const { return arena_; }

 private:
  friend class Arena;
  CallSpine(ClientMetadataHandle client_initial_metadata,
            grpc_event_engine::experimental::EventEngine* event_engine,
            Arena* arena, bool arena_is_owned,
            grpc_call_context_element* legacy_context)
      : Party(1),
        call_filters_(std::move(client_initial_metadata)),
        arena_(arena),
        event_engine_(event_engine),
        arena_is_owned_(arena_is_owned) {
    if (legacy_context == nullptr) {
      legacy_context_ = static_cast<grpc_call_context_element*>(
          arena->Alloc(sizeof(grpc_call_context_element) * GRPC_CONTEXT_COUNT));
      memset(legacy_context_, 0,
             sizeof(grpc_call_context_element) * GRPC_CONTEXT_COUNT);
      legacy_context_is_owned_ = true;
    } else {
      legacy_context_ = legacy_context;
      legacy_context_is_owned_ = false;
    }
  }

  class ScopedContext
      : public ScopedActivity,
        public promise_detail::Context<Arena>,
        public promise_detail::Context<
            grpc_event_engine::experimental::EventEngine>,
        public promise_detail::Context<grpc_call_context_element> {
   public:
    explicit ScopedContext(CallSpine* spine)
        : ScopedActivity(spine),
          Context<Arena>(spine->arena_),
          Context<grpc_event_engine::experimental::EventEngine>(
              spine->event_engine()),
          Context<grpc_call_context_element>(spine->legacy_context_) {}
  };

  bool RunParty() override {
    ScopedContext context(this);
    return Party::RunParty();
  }

  void PartyOver() override {
    Arena* a = arena_;
    {
      ScopedContext context(this);
      CancelRemainingParticipants();
      a->DestroyManagedNewObjects();
    }
    if (!arena_is_owned_) a = nullptr;
    this->~CallSpine();
    if (a != nullptr) a->Destroy();
  }

  // Call filters/pipes part of the spine
  CallFilters call_filters_;
  Arena* const arena_;
  // Event engine associated with this call
  grpc_event_engine::experimental::EventEngine* const event_engine_;
  // Legacy context
  // TODO(ctiller): remove
  grpc_call_context_element* legacy_context_;
  bool legacy_context_is_owned_;
  const bool arena_is_owned_;
};

class CallInitiator {
 public:
  explicit CallInitiator(RefCountedPtr<CallSpine> spine)
      : spine_(std::move(spine)) {}

  auto PullServerInitialMetadata() {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    return spine_->call_filters().PullServerInitialMetadata();
  }

  auto PullServerTrailingMetadata() {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    return spine_->call_filters().PullServerTrailingMetadata();
  }

  auto PullMessage() {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    return spine_->call_filters().PullServerToClientMessage();
  }

  auto PushMessage(MessageHandle message) {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    GPR_DEBUG_ASSERT(message != nullptr);
    return spine_->call_filters().PushClientToServerMessage(std::move(message));
  }

  void FinishSends() {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    spine_->call_filters().FinishClientToServerSends();
  }

  template <typename Promise>
  auto CancelIfFails(Promise promise) {
    return spine_->CancelIfFails(std::move(promise));
  }

  void Cancel() {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    spine_->call_filters().PushServerTrailingMetadata(
        ServerMetadataFromStatus(absl::CancelledError()));
  }

  template <typename PromiseFactory>
  void SpawnGuarded(absl::string_view name, PromiseFactory promise_factory) {
    spine_->SpawnGuarded(name, std::move(promise_factory));
  }

  template <typename PromiseFactory>
  void SpawnInfallible(absl::string_view name, PromiseFactory promise_factory) {
    spine_->SpawnInfallible(name, std::move(promise_factory));
  }

  template <typename PromiseFactory>
  auto SpawnWaitable(absl::string_view name, PromiseFactory promise_factory) {
    return spine_->SpawnWaitable(name, std::move(promise_factory));
  }

  Arena* arena() { return spine_->arena(); }

 private:
  RefCountedPtr<CallSpine> spine_;
};

class CallHandler {
 public:
  explicit CallHandler(RefCountedPtr<CallSpine> spine)
      : spine_(std::move(spine)) {}

  template <typename ContextType>
  void SetContext(ContextType context) {
    // FIXME: implement
  }

  auto PullClientInitialMetadata() {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    return spine_->call_filters().PullClientInitialMetadata();
  }

  auto PushServerInitialMetadata(absl::optional<ServerMetadataHandle> md) {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    return If(
        md.has_value(),
        [&md, this]() {
          return spine_->call_filters().PushServerInitialMetadata(
              std::move(md.value()));
        },
        [this]() {
          spine_->call_filters().NoServerInitialMetadata();
          return []() -> StatusFlag { return Success{}; };
        });
  }

  auto PushServerTrailingMetadata(ServerMetadataHandle md) {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    spine_->call_filters().PushServerTrailingMetadata(std::move(md));
    return Map(spine_->call_filters().WasCancelled(),
               [](bool was_cancelled) { return StatusFlag(!was_cancelled); });
  }

  auto PullMessage() {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    return spine_->call_filters().PullClientToServerMessage();
  }

  auto PushMessage(MessageHandle message) {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    return spine_->call_filters().PushServerToClientMessage(std::move(message));
  }

  void Cancel(ServerMetadataHandle status) {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    spine_->call_filters().PushServerTrailingMetadata(std::move(status));
  }

  template <typename Promise>
  auto CancelIfFails(Promise promise) {
    return spine_->CancelIfFails(std::move(promise));
  }

  auto WasCancelled() {
    GPR_DEBUG_ASSERT(GetContext<Activity>() == spine_.get());
    return spine_->call_filters().WasCancelled();
  }

  template <typename PromiseFactory>
  void SpawnGuarded(absl::string_view name, PromiseFactory promise_factory) {
    spine_->SpawnGuarded(name, std::move(promise_factory));
  }

  template <typename PromiseFactory>
  void SpawnInfallible(absl::string_view name, PromiseFactory promise_factory) {
    spine_->SpawnInfallible(name, std::move(promise_factory));
  }

  template <typename PromiseFactory>
  auto SpawnWaitable(absl::string_view name, PromiseFactory promise_factory) {
    return spine_->SpawnWaitable(name, std::move(promise_factory));
  }

  Arena* arena() { return spine_->arena(); }

  grpc_event_engine::experimental::EventEngine* event_engine() {
    return spine_->event_engine();
  }

  grpc_call_context_element& legacy_context(grpc_context_index index) const {
    return spine_->legacy_context(index);
  }

  std::string DebugTag() { return spine_->DebugTag(); }

 private:
  RefCountedPtr<CallSpine> spine_;
};

class UnstartedCallHandler {
 public:
  explicit UnstartedCallHandler(RefCountedPtr<CallSpine> spine)
      : spine_(std::move(spine)) {}

  // Returns the client initial metadata, which has not yet been
  // processed by the stack that will ultimately be used for this call.
  ClientMetadata& UnprocessedClientInitialMetadata();

  // Starts the call using the specified stack.
  // This must be called only once, and the UnstartedCallHandler object
  // may not be used after this is called.
  CallHandler StartCall(RefCountedPtr<CallFilters::Stack> stack);

  void Cancel(ServerMetadataHandle status) {
    spine_->call_filters().PushServerTrailingMetadata(std::move(status));
  }

  template <typename ContextType>
  void SetContext(ContextType context) {
    // FIXME: implement
  }

  Arena* arena() { return spine_->arena(); }

  Party* party() { return spine_.get(); }

  template <typename Promise>
  auto CancelIfFails(Promise promise) {
    return spine_->CancelIfFails(std::move(promise));
  }

  template <typename PromiseFactory>
  void SpawnGuarded(absl::string_view name, PromiseFactory promise_factory) {
    spine_->SpawnGuarded(name, std::move(promise_factory));
  }

  template <typename PromiseFactory>
  void SpawnInfallible(absl::string_view name, PromiseFactory promise_factory) {
    spine_->SpawnInfallible(name, std::move(promise_factory));
  }

  template <typename PromiseFactory>
  auto SpawnWaitable(absl::string_view name, PromiseFactory promise_factory) {
    return spine_->SpawnWaitable(name, std::move(promise_factory));
  }

 private:
  RefCountedPtr<CallSpine> spine_;
};

struct CallInitiatorAndUnstartedHandler {
  CallInitiator initiator;
  UnstartedCallHandler unstarted_handler;
};

CallInitiatorAndUnstartedHandler MakeCallPair(
    ClientMetadataHandle client_initial_metadata,
    grpc_event_engine::experimental::EventEngine* event_engine, Arena* arena,
    bool arena_is_owned);

template <typename CallHalf>
auto OutgoingMessages(CallHalf h) {
  struct Wrapper {
    CallHalf h;
    auto Next() { return h.PullMessage(); }
  };
  return Wrapper{std::move(h)};
}

// Forward a call from `call_handler` to `call_initiator`
void ForwardCall(CallHandler call_handler, CallInitiator call_initiator);

CallInitiator MakeFailedCall(absl::Status status);

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_TRANSPORT_CALL_SPINE_H
