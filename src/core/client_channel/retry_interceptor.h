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

#ifndef GRPC_SRC_CORE_CLIENT_CHANNEL_RETRY_INTERCEPTOR_H
#define GRPC_SRC_CORE_CLIENT_CHANNEL_RETRY_INTERCEPTOR_H

#include "src/core/call/request_buffer.h"
#include "src/core/client_channel/retry_service_config.h"
#include "src/core/client_channel/retry_throttle.h"
#include "src/core/filter/filter_args.h"
#include "src/core/lib/transport/interception_chain.h"
#include "src/core/util/backoff.h"

// Channel arg key for server URI string.
#define GRPC_ARG_SERVER_URI "grpc.server_uri"

namespace grpc_core {

namespace retry_detail {
class RetryState {
 public:
  RetryState(
      const internal::RetryMethodConfig* retry_policy,
      RefCountedPtr<internal::ServerRetryThrottleData> retry_throttle_data)
      : retry_policy_(retry_policy),
        retry_throttle_data_(std::move(retry_throttle_data)),
        retry_backoff_(
            BackOff::Options()
                .set_initial_backoff(retry_policy_ == nullptr
                                         ? Duration::Zero()
                                         : retry_policy_->initial_backoff())
                .set_multiplier(retry_policy_ == nullptr
                                    ? 0
                                    : retry_policy_->backoff_multiplier())
                // This value was picked arbitrarily.  It can be changed if
                // there is any even moderately compelling reason to do so.
                .set_jitter(0.2)
                .set_max_backoff(retry_policy_ == nullptr
                                     ? Duration::Zero()
                                     : retry_policy_->max_backoff())) {}

  // if nullopt --> commit & don't retry
  // if duration --> retry after duration
  absl::optional<Duration> ShouldRetry(
      const ServerMetadata& md, bool committed,
      absl::FunctionRef<std::string()> lazy_attempt_debug_string);
  int num_attempts_completed() const { return num_attempts_completed_; }

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const RetryState& state) {
    sink.Append(absl::StrCat("policy:{", *state.retry_policy_, "} throttle:",
                             state.retry_throttle_data_ != nullptr,
                             " attempts:", state.num_attempts_completed_));
  }

 private:
  const internal::RetryMethodConfig* const retry_policy_;
  RefCountedPtr<internal::ServerRetryThrottleData> retry_throttle_data_;
  int num_attempts_completed_ = 0;
  BackOff retry_backoff_;
};

absl::StatusOr<RefCountedPtr<internal::ServerRetryThrottleData>>
ServerRetryThrottleDataFromChannelArgs(const ChannelArgs& args);
}  // namespace retry_detail

class RetryInterceptor : public Interceptor {
 public:
  RetryInterceptor(
      const ChannelArgs& args,
      RefCountedPtr<internal::ServerRetryThrottleData> retry_throttle_data);

  static absl::StatusOr<RefCountedPtr<RetryInterceptor>> Create(
      const ChannelArgs& args, const FilterArgs&);

  void Orphaned() override {}

 protected:
  void InterceptCall(UnstartedCallHandler unstarted_call_handler) override;

 private:
  class Attempt;

  class Call : public RefCounted<Call, NonPolymorphicRefCount, UnrefCallDtor> {
   public:
    Call(RefCountedPtr<RetryInterceptor> interceptor, CallHandler call_handler);

    void StartAttempt();
    void Start();

    RequestBuffer* request_buffer() { return &request_buffer_; }
    CallHandler* call_handler() { return &call_handler_; }
    RetryInterceptor* interceptor() { return interceptor_.get(); }
    // if nullopt --> commit & don't retry
    // if duration --> retry after duration
    absl::optional<Duration> ShouldRetry(
        const ServerMetadata& md,
        absl::FunctionRef<std::string()> lazy_attempt_debug_string) {
      return retry_state_.ShouldRetry(md, request_buffer_.committed(),
                                      lazy_attempt_debug_string);
    }
    int num_attempts_completed() const {
      return retry_state_.num_attempts_completed();
    }

    std::string DebugTag();

   private:
    void MaybeCommit(size_t buffered);
    auto ClientToBuffer();

    RequestBuffer request_buffer_;
    CallHandler call_handler_;
    RefCountedPtr<RetryInterceptor> interceptor_;
    RefCountedPtr<Attempt> current_attempt_;
    retry_detail::RetryState retry_state_;
  };

  class Attempt
      : public RefCounted<Attempt, NonPolymorphicRefCount, UnrefCallDtor> {
   public:
    explicit Attempt(RefCountedPtr<Call> call);

    void Start();
    void Cancel();
    void Commit();
    RequestBuffer::Reader* reader() { return &reader_; }

    std::string DebugTag() const;

   private:
    auto ServerToClient();
    auto ServerToClientGotInitialMetadata(ServerMetadataHandle md);
    auto ServerToClientGotTrailersOnlyResponse();

    RequestBuffer::Reader reader_;
    RefCountedPtr<Call> call_;
    CallInitiator initiator_;
  };

  const internal::RetryMethodConfig* GetRetryPolicy();

  const size_t per_rpc_retry_buffer_size_;
  const size_t service_config_parser_index_;
  const RefCountedPtr<internal::ServerRetryThrottleData> retry_throttle_data_;
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_CLIENT_CHANNEL_RETRY_INTERCEPTOR_H
