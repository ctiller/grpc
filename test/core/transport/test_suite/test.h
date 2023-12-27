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

#ifndef TESTSUITETEST_H
#define TESTSUITETEST_H

#include <initializer_list>
#include <memory>
#include <queue>

#include "absl/functional/any_invocable.h"
#include "absl/strings/string_view.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "src/core/lib/gprpp/time.h"
#include "src/core/lib/iomgr/timer_manager.h"
#include "src/core/lib/promise/cancel_callback.h"
#include "src/core/lib/resource_quota/resource_quota.h"
#include "test/core/event_engine/fuzzing_event_engine/fuzzing_event_engine.h"
#include "test/core/event_engine/fuzzing_event_engine/fuzzing_event_engine.pb.h"
#include "test/core/transport/test_suite/fixture.h"

namespace grpc_core {

class TransportTest : public ::testing::Test {
 protected:
  TransportTest(std::unique_ptr<TransportFixture> fixture)
      : fixture_(std::move(fixture)) {}

  void RunTest();

  void SetServerAcceptor();
  CallInitiator CreateCall();

  CallHandler TickUntilServerCall();
  void WaitForAllPendingWork();

  struct NameAndLocation {
    NameAndLocation(const char* name, SourceLocation location = {})
        : location_(location), name_(name) {}
    NameAndLocation Next() const {
      return NameAndLocation(name_, location_, step_ + 1);
    }

    SourceLocation location() const { return location_; }
    absl::string_view name() const { return name_; }
    int step() const { return step_; }

   private:
    NameAndLocation(absl::string_view name, SourceLocation location, int step)
        : location_(location), name_(name), step_(step) {}
    SourceLocation location_;
    absl::string_view name_;
    int step_ = 1;
  };

 private:
  class ActionState {
   public:
    enum State : uint8_t {
      kNotCreated,
      kNotStarted,
      kStarted,
      kDone,
      kCancelledAfterStart,
    };

    ActionState(NameAndLocation name_and_location, State state);

    State Get() const { return state_; }
    void Set(State state) { state_ = state; }
    const NameAndLocation& name_and_location() const {
      return name_and_location_;
    }
    SourceLocation location() const { return name_and_location().location(); }
    const char* file() const { return location().file(); }
    int line() const { return location().line(); }
    absl::string_view name() const { return name_and_location().name(); }
    int step() const { return name_and_location().step(); }
    bool IsDone();

   private:
    const NameAndLocation name_and_location_;
    std::atomic<State> state_;
  };

  template <typename Promise>
  static auto WrapPromise(Promise promise,
                          const std::shared_ptr<ActionState>& state) {
    return OnCancel(
        [state, promise = promise_detail::PromiseLike<Promise>(
                    std::move(promise))]() mutable {
          state->Set(ActionState::State::kStarted);
          auto result = promise();
          if (result.ready()) {
            state->Set(ActionState::State::kDone);
          }
          return result;
        },
        [state]() mutable {
          state->Set(ActionState::State::kCancelledAfterStart);
        });
  }

  template <typename Promise>
  auto WrapPromise(Promise promise, NameAndLocation name_and_location) {
    auto state = std::make_shared<ActionState>(name_and_location,
                                               ActionState::kNotStarted);
    pending_actions_.push(state);
    return WrapPromise(std::move(promise), std::move(state));
  }

  template <typename Arg, typename PromiseFactory>
  auto WrapPromiseFactory(PromiseFactory promise_factory,
                          NameAndLocation name_and_location) {
    class Wrapper {
     public:
      Wrapper(PromiseFactory promise_factory,
              std::shared_ptr<ActionState> state)
          : promise_state_(Factory(std::move(promise_factory))),
            action_state_(std::move(state)) {}

      void Start(Arg arg) {
        action_state_->Set(ActionState::State::kNotStarted);
        promise_state_.template emplace<Promise>(
            WrapPromise(absl::get<Factory>(promise_state_).Make(std::move(arg)),
                        action_state_));
      }

      auto Continue() { return absl::get<Promise>(promise_state_)(); }

     private:
      using Factory = promise_detail::OncePromiseFactory<Arg, PromiseFactory>;
      using PromiseFromFactory = typename Factory::Promise;
      using Promise = decltype(WrapPromise(std::declval<PromiseFromFactory>(),
                                           std::shared_ptr<ActionState>()));
      using PromiseState = absl::variant<Factory, Promise>;
      PromiseState promise_state_;
      std::shared_ptr<ActionState> action_state_;
    };

    auto state = std::make_shared<ActionState>(name_and_location,
                                               ActionState::kNotCreated);
    pending_actions_.push(state);
    return Wrapper(std::move(promise_factory), std::move(state));
  }

  template <typename FirstWrappedAction>
  auto Append(NameAndLocation name_and_location,
              FirstWrappedAction first_wrapped_action) {
    return first_wrapped_action;
  }

  template <typename FirstWrappedAction, typename FirstFollowUpAction,
            typename... FollowUps>
  auto Append(NameAndLocation name_and_location,
              FirstWrappedAction first_wrapped_action,
              FirstFollowUpAction first_follow_up_action,
              FollowUps... follow_up_actions) {
    auto follow_up = WrapPromiseFactory<
        typename PollTraits<decltype(first_wrapped_action())>::Type>(
        std::move(first_follow_up_action), name_and_location);
    using FollowUpResult = decltype(follow_up.Continue());
    return Append(
        name_and_location.Next(),
        [first_done = false,
         first_wrapped_action = std::move(first_wrapped_action),
         follow_up = std::move(follow_up)]() mutable -> FollowUpResult {
          if (!first_done) {
            auto result = first_wrapped_action();
            if (result.ready()) {
              follow_up.Start(std::move(result.value()));
              first_done = true;
            } else {
              return Pending{};
            }
          }
          return follow_up.Continue();
        },
        std::move(follow_up_actions)...);
  }

 protected:
  template <typename Context, typename FirstAction, typename... FollowUps>
  void SpawnTestSeq(Context& context, NameAndLocation name_and_location,
                    FirstAction first_action, FollowUps... follow_ups) {
    class Wrapper {
     public:
      Wrapper(FirstAction promise_factory, std::shared_ptr<ActionState> state)
          : promise_state_(Factory(std::move(promise_factory))),
            action_state_(std::move(state)) {}

      void Start() {
        action_state_->Set(ActionState::State::kNotStarted);
        promise_state_.template emplace<Promise>(WrapPromise(
            absl::get<Factory>(promise_state_).Make(), action_state_));
      }

      auto Continue() { return absl::get<Promise>(promise_state_)(); }

     private:
      using Factory = promise_detail::OncePromiseFactory<void, FirstAction>;
      using PromiseFromFactory = typename Factory::Promise;
      using Promise = decltype(WrapPromise(std::declval<PromiseFromFactory>(),
                                           std::shared_ptr<ActionState>()));
      using PromiseState = absl::variant<Factory, Promise>;
      PromiseState promise_state_;
      std::shared_ptr<ActionState> action_state_;
    };

    auto state = std::make_shared<ActionState>(name_and_location,
                                               ActionState::kNotCreated);
    pending_actions_.push(state);

    context.SpawnInfallible(
        name_and_location.name(),
        Append(
            name_and_location.Next(),
            [wrapper = Wrapper(std::move(first_action), state),
             started = false]() mutable {
              if (!started) {
                wrapper.Start();
                started = true;
              }
              return wrapper.Continue();
            },
            std::move(follow_ups)...));
  }

 private:
  virtual void TestImpl() = 0;

  void Timeout();

  class Acceptor final : public ServerTransport::Acceptor {
   public:
    Acceptor(grpc_event_engine::experimental::EventEngine* event_engine,
             MemoryAllocator* allocator)
        : event_engine_(event_engine), allocator_(allocator) {}

    Arena* CreateArena() override;
    absl::StatusOr<CallInitiator> CreateCall(
        ClientMetadata& client_initial_metadata, Arena* arena) override;
    absl::optional<CallHandler> PopHandler();

   private:
    std::queue<CallHandler> handlers_;
    grpc_event_engine::experimental::EventEngine* const event_engine_;
    MemoryAllocator* const allocator_;
  };

  class WatchDog {
   public:
    explicit WatchDog(TransportTest* test) : test_(test) {}
    ~WatchDog() { test_->event_engine_->Cancel(timer_); }

   private:
    TransportTest* const test_;
    grpc_event_engine::experimental::EventEngine::TaskHandle const timer_{
        test_->event_engine_->RunAfter(Duration::Minutes(5),
                                       [this]() { test_->Timeout(); })};
  };

  std::unique_ptr<TransportFixture> fixture_;
  TransportFixture::ClientAndServerTransportPair transport_pair_ =
      fixture_->CreateTransportPair();
  std::shared_ptr<grpc_event_engine::experimental::FuzzingEventEngine>
      event_engine_{
          std::make_shared<grpc_event_engine::experimental::FuzzingEventEngine>(
              []() {
                grpc_timer_manager_set_threading(false);
                grpc_event_engine::experimental::FuzzingEventEngine::Options
                    options;
                return options;
              }(),
              fuzzing_event_engine::Actions())};
  MemoryAllocator allocator_ = MakeResourceQuota("test-quota")
                                   ->memory_quota()
                                   ->CreateMemoryAllocator("test-allocator");
  Acceptor acceptor_{event_engine_.get(), &allocator_};
  std::queue<std::shared_ptr<ActionState>> pending_actions_;
};

class TransportTestRegistry {
 public:
  static TransportTestRegistry& Get();
  void RegisterTest(absl::string_view name,
                    absl::AnyInvocable<TransportTest*(
                        std::unique_ptr<grpc_core::TransportFixture>) const>
                        create);

  struct Test {
    absl::string_view name;
    absl::AnyInvocable<TransportTest*(
        std::unique_ptr<grpc_core::TransportFixture>) const>
        create;
  };

  const std::vector<Test>& tests() const { return tests_; }

 private:
  std::vector<Test> tests_;
};

}  // namespace grpc_core

#define TRANSPORT_TEST(name)                                                 \
  class TransportTest_##name : public grpc_core::TransportTest {             \
   public:                                                                   \
    using TransportTest::TransportTest;                                      \
    void TestBody() override { RunTest(); }                                  \
                                                                             \
   private:                                                                  \
    void TestImpl() override;                                                \
    static grpc_core::TransportTest* Create(                                 \
        std::unique_ptr<grpc_core::TransportFixture> fixture) {              \
      return new TransportTest_##name(std::move(fixture));                   \
    }                                                                        \
    static int registered_;                                                  \
  };                                                                         \
  int TransportTest_##name::registered_ =                                    \
      (grpc_core::TransportTestRegistry::Get().RegisterTest(#name, &Create), \
       0);                                                                   \
  void TransportTest_##name::TestImpl()

#endif
