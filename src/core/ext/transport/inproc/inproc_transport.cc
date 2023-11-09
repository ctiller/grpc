// Copyright 2017 gRPC authors.
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

#include <grpc/support/port_platform.h>

#include "src/core/ext/transport/inproc/inproc_transport.h"

#include <atomic>

#include <grpc/support/log.h>

#include "src/core/ext/transport/inproc/legacy_inproc_transport.h"
#include "src/core/lib/experiments/experiments.h"
#include "src/core/lib/gprpp/crash.h"
#include "src/core/lib/transport/transport.h"

namespace grpc_core {

namespace {
class InprocServerTransport final : public Transport, public ServerTransport {
 public:
  void SetAcceptFunction(AcceptFunction accept_function) override {
    accept_ = std::move(accept_function);
    ConnectionState expect = ConnectionState::kInitial;
    state_.compare_exchange_strong(expect, ConnectionState::kReady,
                                   std::memory_order_acq_rel,
                                   std::memory_order_acquire);
  }

  void Orphan() override { grpc_core::Crash("unimplemented"); }

  FilterStackTransport* filter_stack_transport() override { return nullptr; }
  ClientTransport* client_transport() override { return nullptr; }
  ServerTransport* server_transport() override { return this; }
  absl::string_view GetTransportName() const override { return "inproc"; }
  void SetPollset(grpc_stream* stream, grpc_pollset* pollset) override {}
  void SetPollsetSet(grpc_stream* stream,
                     grpc_pollset_set* pollset_set) override {}
  void PerformOp(grpc_transport_op* op) override { Crash("unimplemented"); }
  grpc_endpoint* GetEndpoint() override { return nullptr; }

  void Disconnect() {
    state_.store(ConnectionState::kDisconnected, std::memory_order_relaxed);
  }

  CallInitiator AcceptCall(ClientMetadata& md);

 private:
  enum class ConnectionState : uint8_t { kInitial, kReady, kDisconnected };

  std::atomic<ConnectionState> state_{ConnectionState::kInitial};
  AcceptFunction accept_;
};

class InprocClientTransport final : public Transport, public ClientTransport {
 public:
  ~InprocClientTransport() { server_transport_->Disconnect(); }

  void StartCall(CallHandler call_handler) override {
    call_handler.Spawn(
        TrySeq(call_handler.PullClientInitialMetadata(),
               [server_transport = server_transport_,
                call_handler](ClientMetadataHandle md) {
                 auto call_initiator = server_transport->AcceptCall(*md);
                 ForwardCall(std::move(call_handler), std::move(call_initiator),
                             std::move(md));
                 return StatusFlag(true);
               }));
  }

  void Orphan() override { grpc_core::Crash("unimplemented"); }

  FilterStackTransport* filter_stack_transport() override { return nullptr; }
  ClientTransport* client_transport() override { return this; }
  ServerTransport* server_transport() override { return nullptr; }
  absl::string_view GetTransportName() const override { return "inproc"; }
  void SetPollset(grpc_stream* stream, grpc_pollset* pollset) override {}
  void SetPollsetSet(grpc_stream* stream,
                     grpc_pollset_set* pollset_set) override {}
  void PerformOp(grpc_transport_op* op) override { Crash("unimplemented"); }
  grpc_endpoint* GetEndpoint() override { return nullptr; }

 private:
  RefCountedPtr<InprocServerTransport> server_transport_ =
      MakeRefCounted<InprocServerTransport>();
};

bool UsePromiseBasedTransport() {
  if (!IsPromiseBasedInprocTransportEnabled()) return false;
  if (!IsPromiseBasedClientCallEnabled()) {
    gpr_log(GPR_ERROR,
            "Promise based inproc transport requested but promise based client "
            "calls are disabled: using legacy implementation.");
    return false;
  }
  if (!IsPromiseBasedServerCallEnabled()) {
    gpr_log(GPR_ERROR,
            "Promise based inproc transport requested but promise based server "
            "calls are disabled: using legacy implementation.");
    return false;
  }
  return true;
}
}  // namespace

}  // namespace grpc_core

grpc_channel* grpc_inproc_channel_create(grpc_server* server,
                                         const grpc_channel_args* args,
                                         void* reserved) {
  if (!grpc_core::UsePromiseBasedTransport()) {
    return grpc_legacy_inproc_channel_create(server, args, reserved);
  }
  grpc_core::Crash("unimplemented");
}
