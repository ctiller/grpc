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

#ifndef CHTTP2_H
#define CHTTP2_H

#include "absl/container/flat_hash_map.h"
#include "frame.h"
#include "hpack_encoder.h"

#include "src/core/ext/transport/chttp2/transport/http2_settings.h"
#include "src/core/lib/promise/wait_set.h"
#include "src/core/lib/transport/transport.h"

namespace grpc_core {

template <typename T>
class MultiPromiseState {
 public:
  // Given a promise factory F taking a reference to state_,
  // return a promise factory that returns a promise that locks state_ before
  // polling, then polls the promise returned by F, then unlocks state_.
  template <typename F>
  auto WithLock(F&& f) {
    using Factory = promise_detail::OncePromiseFactory<T&, F>;
    using Promise = typename Factory::Promise;
    class WithLockPromise {
     public:
      WithLockPromise(MultiPromiseState* state, F&& f) : state_(state) {
        Construct(&f_, std::move(f));
      }
      ~WithLockPromise() {
        if (!started_) {
          Destruct(&f_);
        } else {
          Destruct(&p_);
        }
      }
      WithLockPromise(const WithLockPromise&) = delete;
      WithLockPromise& operator=(const WithLockPromise&) = delete;
      WithLockPromise(WithLockPromise&& other) noexcept
          : state_(other.state_), started_(other.started_) {
        GPR_ASSERT(!started_);
        Construct(&f_, std::move(other.f_));
      }
      WithLockPromise& operator=(WithLockPromise&& other) noexcept = delete;

      auto operator()() {
        MutexLock lock(&state_->mu_);
        if (!started_) {
          auto p = f_.Make(state_->state_);
          started_ = true;
          Destruct(&f_);
          Construct(&p_, std::move(p));
        }
        return p_();
      }

     private:
      // Prior to polling: the parent state
      // During polling: nullptr
      MultiPromiseState* state_;
      bool started_ = false;
      union {
        Factory f_;
        Promise p_;
      };
    };
    return WithLockPromise(this, std::forward<F>(f));
  }

 private:
  Mutex mu_;
  T state_ ABSL_GUARDED_BY(mu_);
};

class Chttp2ClientTransport final : public Transport, public ClientTransport {
 public:
  Chttp2ClientTransport();

  FilterStackTransport* filter_stack_transport() override { return nullptr; }
  ClientTransport* client_transport() override { return this; }
  ServerTransport* server_transport() override { return nullptr; }
  absl::string_view GetTransportName() const override { return "chaotic_good"; }
  void SetPollset(grpc_stream*, grpc_pollset*) override {}
  void SetPollsetSet(grpc_stream*, grpc_pollset_set*) override {}
  void PerformOp(grpc_transport_op*) override { Crash("unimplemented"); }
  grpc_endpoint* GetEndpoint() override { return nullptr; }
  void Orphan() override { delete this; }

  void StartCall(CallHandler call_handler) override;

 private:
  // Represent one allocated stream id.
  class StreamId {
   public:
    explicit StreamId(uint32_t id) : id_(id) {}
    uint32_t id() const { return id_; }
    bool operator==(const StreamId& other) const { return id_ == other.id_; }
    bool operator!=(const StreamId& other) const { return !(*this == other); }
    bool operator<(const StreamId& other) const { return id_ < other.id_; }
    bool is_client_id() const { return id_ % 2 == 1; }

    template <typename H>
    friend H AbslHashValue(H h, StreamId s) {
      return H::combine(std::move(h), s.id_);
    }

   private:
    uint32_t id_;
  };

  class StreamMap {
   public:
    auto AllocateStreamId(CallHandler call_handler);
    void UpdateMaxConcurrentStreams(uint32_t max_concurrent_streams);
    void DropStreamId(StreamId stream_id);

   private:
    absl::flat_hash_map<StreamId, CallHandler> streams_;
    WaitSet allocate_stream_id_wait_set_;
    uint32_t max_concurrent_streams_;
    uint32_t next_stream_id_ = 1;
  };

  class WriteQueue {
   public:
    void Push(Http2Frame frame);
    std::vector<Http2Frame>& PushBuffer();

   private:
    std::vector<Http2Frame> frames_;
  };

  struct TransportState {
    StreamMap stream_map;
    WriteQueue write_queue;
    Http2SettingsManager settings;
    HPackCompressor hpack_encoder;
  };

  auto CallOutboundLoop(CallHandler call_handler);

  MultiPromiseState<TransportState> state_;
};

}  // namespace grpc_core

#endif
