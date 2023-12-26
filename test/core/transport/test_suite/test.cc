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

#include "test/core/transport/test_suite/test.h"

namespace grpc_core {

void TransportTest::SetServerAcceptor() {
  transport_pair_.server->server_transport()->SetAcceptor(&acceptor_);
}

CallInitiator TransportTest::CreateCall() {
  auto call = MakeCall(event_engine_.get(), Arena::Create(1024, &allocator_));
  call.handler.SpawnInfallible("start-call", [this, handler = call.handler]() {
    transport_pair_.client->client_transport()->StartCall(handler);
    return Empty{};
  });
  return std::move(call.initiator);
}

TransportTestRegistry& TransportTestRegistry::Get() {
  static TransportTestRegistry* registry = new TransportTestRegistry();
  return *registry;
}

void TransportTestRegistry::RegisterTest(
    absl::string_view name,
    absl::AnyInvocable<
        TransportTest*(std::unique_ptr<grpc_core::TransportFixture>) const>
        create) {
  tests_.push_back({name, std::move(create)});
}

void TransportTest::RunTest() {
  TestImpl();
  EXPECT_EQ(pending_actions_.size(), 0)
      << "There are still pending actions: did you forget to call "
         "WaitForAllPendingWork()?";
  event_engine_->TickUntilIdle();
  event_engine_->UnsetGlobalHooks();
}

Arena* TransportTest::Acceptor::CreateArena() {
  return Arena::Create(1024, allocator_);
}

absl::StatusOr<CallInitiator> TransportTest::Acceptor::CreateCall(
    ClientMetadata& client_initial_metadata, Arena* arena) {
  auto call = MakeCall(event_engine_, arena);
  handlers_.push(std::move(call.handler));
  return std::move(call.initiator);
}

absl::optional<CallHandler> TransportTest::Acceptor::PopHandler() {
  if (!handlers_.empty()) {
    auto handler = std::move(handlers_.front());
    handlers_.pop();
    return handler;
  }
  return absl::nullopt;
}

}  // namespace grpc_core
