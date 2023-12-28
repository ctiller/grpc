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

///////////////////////////////////////////////////////////////////////////////
// TransportTestRegistry

TransportTestRegistry& TransportTestRegistry::Get() {
  static TransportTestRegistry* registry = new TransportTestRegistry();
  return *registry;
}

void TransportTestRegistry::RegisterTest(
    absl::string_view name, absl::AnyInvocable<TransportTest*(
                                std::unique_ptr<grpc_core::TransportFixture>,
                                const fuzzing_event_engine::Actions&) const>
                                create) {
  tests_.push_back({name, std::move(create)});
}

///////////////////////////////////////////////////////////////////////////////
// TransportTest

void TransportTest::RunTest() {
  TestImpl();
  EXPECT_EQ(pending_actions_.size(), 0)
      << "There are still pending actions: did you forget to call "
         "WaitForAllPendingWork()?";
  event_engine_->TickUntilIdle();
  event_engine_->UnsetGlobalHooks();
}

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

CallHandler TransportTest::TickUntilServerCall() {
  WatchDog watchdog(this);
  for (;;) {
    auto handler = acceptor_.PopHandler();
    if (handler.has_value()) return std::move(*handler);
    event_engine_->Tick();
  }
}

void TransportTest::WaitForAllPendingWork() {
  WatchDog watchdog(this);
  while (!pending_actions_.empty()) {
    if (pending_actions_.front()->IsDone()) {
      pending_actions_.pop();
      continue;
    }
    event_engine_->Tick();
  }
}

void TransportTest::Timeout() {
  std::vector<std::string> lines;
  lines.emplace_back("Timeout waiting for pending actions to complete");
  while (!pending_actions_.empty()) {
    auto action = std::move(pending_actions_.front());
    pending_actions_.pop();
    absl::string_view state_name;
    switch (action->Get()) {
      case transport_test_detail::ActionState::kNotCreated:
        state_name = "[!created]";
        break;
      case transport_test_detail::ActionState::kNotStarted:
        state_name = "[!started]";
        break;
      case transport_test_detail::ActionState::kStarted:
        state_name = "[pending ]";
        break;
      case transport_test_detail::ActionState::kDone:
      case transport_test_detail::ActionState::kCancelled:
        continue;
    }
    absl::string_view file_name = action->file();
    auto pos = file_name.find_last_of('/');
    if (pos != absl::string_view::npos) {
      file_name = file_name.substr(pos + 1);
    }
    lines.emplace_back(absl::StrCat("  ", state_name, " ", action->name(), " [",
                                    action->step(), "]: ", file_name, ":",
                                    action->line()));
  }
  Crash(absl::StrJoin(lines, "\n"));
}

///////////////////////////////////////////////////////////////////////////////
// TransportTest::Acceptor

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

///////////////////////////////////////////////////////////////////////////////
// ActionState

namespace transport_test_detail {

ActionState::ActionState(NameAndLocation name_and_location)
    : name_and_location_(name_and_location), state_(kNotCreated) {}

bool ActionState::IsDone() {
  switch (state_) {
    case kNotCreated:
    case kNotStarted:
    case kStarted:
      return false;
    case kDone:
    case kCancelled:
      return true;
  }
}

}  // namespace transport_test_detail

}  // namespace grpc_core
