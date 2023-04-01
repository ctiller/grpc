//
//
// Copyright 2015 gRPC authors.
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
//
//

#ifndef GRPC_TEST_CORE_END2END_END2END_TESTS_H
#define GRPC_TEST_CORE_END2END_END2END_TESTS_H

#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <memory>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "gtest/gtest.h"

#include <grpc/byte_buffer.h>
#include <grpc/compression.h>
#include <grpc/grpc.h>
#include <grpc/grpc_security.h>
#include <grpc/impl/propagation_bits.h>
#include <grpc/status.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/time.h>

#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/gprpp/bitset.h"
#include "src/core/lib/gprpp/debug_location.h"
#include "src/core/lib/gprpp/no_destruct.h"
#include "src/core/lib/gprpp/time.h"
#include "src/core/lib/slice/slice.h"
#include "src/core/lib/slice/slice_internal.h"
#include "src/core/lib/surface/call_test_only.h"
#include "src/core/lib/surface/channel.h"
#include "test/core/end2end/cq_verifier.h"
#include "test/core/util/test_config.h"

// Test feature flags.
#define FEATURE_MASK_DOES_NOT_SUPPORT_RETRY 1
#define FEATURE_MASK_SUPPORTS_HOSTNAME_VERIFICATION 2
// Feature mask supports call credentials with a minimum security level of
// GRPC_PRIVACY_AND_INTEGRITY.
#define FEATURE_MASK_SUPPORTS_PER_CALL_CREDENTIALS 4
// Feature mask supports call credentials with a minimum security level of
// GRPC_SECURTITY_NONE.
#define FEATURE_MASK_SUPPORTS_PER_CALL_CREDENTIALS_LEVEL_INSECURE 8
#define FEATURE_MASK_SUPPORTS_REQUEST_PROXYING 16
#define FEATURE_MASK_SUPPORTS_CLIENT_CHANNEL 32
#define FEATURE_MASK_IS_HTTP2 64
#define FEATURE_MASK_ENABLES_TRACES 128
#define FEATURE_MASK_1BYTE_AT_A_TIME 256
#define FEATURE_MASK_DOES_NOT_SUPPORT_WRITE_BUFFERING 512
#define FEATURE_MASK_DOES_NOT_SUPPORT_CLIENT_HANDSHAKE_COMPLETE_FIRST 1024
#define FEATURE_MASK_IS_MINSTACK 2048
#define FEATURE_MASK_IS_SECURE 4096

#define FAIL_AUTH_CHECK_SERVER_ARG_NAME "fail_auth_check"

namespace grpc_core {
class CoreTestFixture {
 public:
  virtual ~CoreTestFixture() {
    ShutdownServer();
    ShutdownClient();
    grpc_completion_queue_shutdown(cq());
    DrainCq();
    grpc_completion_queue_destroy(cq());
  }

  grpc_completion_queue* cq() { return cq_; }
  grpc_server* server() { return server_; }
  grpc_channel* client() { return client_; }

  void InitServer(const ChannelArgs& args) {
    if (server_ != nullptr) ShutdownServer();
    server_ = MakeServer(args);
    GPR_ASSERT(server_ != nullptr);
  }
  void InitClient(const ChannelArgs& args) {
    if (client_ != nullptr) ShutdownClient();
    client_ = MakeClient(args);
    GPR_ASSERT(client_ != nullptr);
  }

  void ShutdownServer() {
    if (server_ == nullptr) return;
    grpc_server_shutdown_and_notify(server_, cq_, server_);
    grpc_event ev;
    do {
      ev = grpc_completion_queue_next(cq_, grpc_timeout_seconds_to_deadline(5),
                                      nullptr);
    } while (ev.type != GRPC_OP_COMPLETE || ev.tag != server_);
    DestroyServer();
  }

  void DestroyServer() {
    if (server_ == nullptr) return;
    grpc_server_destroy(server_);
    server_ = nullptr;
  }

  void ShutdownClient() {
    if (client_ == nullptr) return;
    grpc_channel_destroy(client_);
    client_ = nullptr;
  }

  virtual grpc_server* MakeServer(const ChannelArgs& args) = 0;
  virtual grpc_channel* MakeClient(const ChannelArgs& args) = 0;

 protected:
  void SetServer(grpc_server* server);
  void SetClient(grpc_channel* client);

 private:
  void DrainCq() {
    grpc_event ev;
    do {
      ev = grpc_completion_queue_next(cq_, grpc_timeout_seconds_to_deadline(5),
                                      nullptr);
    } while (ev.type != GRPC_QUEUE_SHUTDOWN);
  }

  grpc_completion_queue* cq_ = grpc_completion_queue_create_for_next(nullptr);
  grpc_server* server_ = nullptr;
  grpc_channel* client_ = nullptr;
};

Slice RandomSlice(size_t length);
Slice RandomBinarySlice(size_t length);
using ByteBufferUniquePtr =
    std::unique_ptr<grpc_byte_buffer, void (*)(grpc_byte_buffer*)>;
ByteBufferUniquePtr ByteBufferFromSlice(Slice slice);

struct CoreTestConfiguration {
  // A descriptive name for this test fixture.
  const char* name;

  // Which features are supported by this fixture. See feature flags above.
  uint32_t feature_mask;

  // If the call host is setup by the fixture (for example, via the
  // GRPC_SSL_TARGET_NAME_OVERRIDE_ARG channel arg), which value should the
  // test expect to find in call_details.host
  const char* overridden_call_host;

  std::function<std::unique_ptr<CoreTestFixture>(
      const ChannelArgs& client_args, const ChannelArgs& server_args)>
      create_fixture;
};

// Base class for e2e tests.
//
// Initialization:
//
// At the start of a test, nothing is initialized. CoreConfiguration is reset,
// and there's certainly no server nor client.
// A test can then register whatever builders it wants into the
// CoreConfiguration and have them picked up. If it does not, it will get the
// default CoreConfiguration.
//
// The test may choose to then create a client and server with InitClient() and
// InitServer(). It does not matter which order they are called, nor whether one
// or both are called. It's necessary to call these if the test demands that
// non-empty channel args should be passed to either the client or server.
//
// If a test does not call InitClient() or InitServer(), then upon the first
// call to either NewClientCall() or NewServerCall(), the client and server will
// be instantiated - this saves substantial boilerplate in the most common case
// for our tests.
//
// Notes:
// - older compilers fail matching absl::string_view with some gmock matchers on
//   older compilers, and it's tremendously convenient to be able to do so. So
//   we use std::string for return types here - performance isn't particularly
//   important, so an extra copy is fine.
class CoreEnd2endTest
    : public ::testing::TestWithParam<const CoreTestConfiguration*> {
 public:
  void SetUp() override;
  void TearDown() override;

  class Call;
  struct RegisteredCall {
    void* p;
  };

  // CallBuilder - results in a call to either grpc_channel_create_call or
  // grpc_channel_create_registered_call.
  // Affords a fluent interface to specify optional arguments.
  class ClientCallBuilder {
   public:
    ClientCallBuilder(CoreEnd2endTest& test, std::string method)
        : test_(test),
          call_selector_(UnregisteredCall{std::move(method), absl::nullopt}) {}
    ClientCallBuilder(CoreEnd2endTest& test, RegisteredCall registered_call)
        : test_(test), call_selector_(registered_call.p) {}

    // Specify the host (otherwise nullptr is passed)
    ClientCallBuilder& Host(std::string host) {
      absl::get<UnregisteredCall>(call_selector_).host = std::move(host);
      return *this;
    }
    // Specify the timeout (otherwise gpr_inf_future is passed) - this time is
    // scaled according to the test environment.
    ClientCallBuilder& Timeout(Duration timeout) {
      if (timeout == Duration::Infinity()) {
        deadline_ = gpr_inf_future(GPR_CLOCK_REALTIME);
        return *this;
      }
      deadline_ = grpc_timeout_milliseconds_to_deadline(timeout.millis());
      return *this;
    }
    // Finally create the call.
    Call Create();

   private:
    CoreEnd2endTest& test_;
    struct UnregisteredCall {
      std::string method;
      absl::optional<std::string> host;
    };
    absl::variant<void*, UnregisteredCall> call_selector_;
    grpc_call* parent_call_ = nullptr;
    uint32_t propagation_mask_ = GRPC_PROPAGATE_DEFAULTS;
    gpr_timespec deadline_ = gpr_inf_future(GPR_CLOCK_REALTIME);
  };

  // Receiving container for incoming metadata.
  class IncomingMetadata {
   public:
    IncomingMetadata() = default;
    ~IncomingMetadata() {
      if (metadata_ != nullptr) grpc_metadata_array_destroy(metadata_.get());
    }

    // Lookup a metadata value by key.
    absl::optional<std::string> Get(absl::string_view key) const;

    // Make a GRPC_RECV_INITIAL_METADATA op - intended for the framework, not
    // for tests.
    grpc_op MakeOp();

   private:
    std::unique_ptr<grpc_metadata_array> metadata_ =
        std::make_unique<grpc_metadata_array>(
            grpc_metadata_array{0, 0, nullptr});
  };

  // Receiving container for one incoming message.
  class IncomingMessage {
   public:
    IncomingMessage() = default;
    IncomingMessage(const IncomingMessage&) = delete;
    IncomingMessage& operator=(const IncomingMessage&) = delete;
    ~IncomingMessage() {
      if (payload_ != nullptr) grpc_byte_buffer_destroy(payload_);
    }

    // Get the payload of the message - concatenated together into a string for
    // easy verification.
    std::string payload() const;
    // Check if the message is the end of the stream.
    bool is_end_of_stream() const { return payload_ == nullptr; }
    // Get the type of the message.
    grpc_byte_buffer_type byte_buffer_type() const { return payload_->type; }
    // Get the compression algorithm used for the message.
    grpc_compression_algorithm compression() const {
      return payload_->data.raw.compression;
    }

    // Make a GRPC_OP_RECV_MESSAGE op - intended for the framework, not for
    // tests.
    grpc_op MakeOp();

   private:
    grpc_byte_buffer* payload_ = nullptr;
  };

  // Receiving container for incoming status on the client from the server.
  class IncomingStatusOnClient {
   public:
    IncomingStatusOnClient() = default;
    IncomingStatusOnClient(const IncomingStatusOnClient&) = delete;
    IncomingStatusOnClient& operator=(const IncomingStatusOnClient&) = delete;
    IncomingStatusOnClient(IncomingStatusOnClient&& other) noexcept = default;
    IncomingStatusOnClient& operator=(IncomingStatusOnClient&& other) noexcept =
        default;
    ~IncomingStatusOnClient() {
      if (data_ != nullptr) {
        grpc_metadata_array_destroy(&data_->trailing_metadata);
        gpr_free(const_cast<char*>(data_->error_string));
      }
    }

    // Get the status code.
    grpc_status_code status() const { return data_->status; }
    // Get the status details.
    std::string message() const {
      return std::string(data_->status_details.as_string_view());
    }
    // Get the error string.
    std::string error_string() const {
      return data_->error_string == nullptr ? "" : data_->error_string;
    }
    // Get a trailing metadata value by key.
    absl::optional<std::string> GetTrailingMetadata(
        absl::string_view key) const;

    // Make a GRPC_OP_RECV_STATUS_ON_CLIENT op - intended for the framework, not
    // for tests.
    grpc_op MakeOp();

   private:
    struct Data {
      grpc_metadata_array trailing_metadata{0, 0, nullptr};
      grpc_status_code status;
      Slice status_details;
      const char* error_string = nullptr;
    };
    std::unique_ptr<Data> data_ = std::make_unique<Data>();
  };

  // Receiving container for incoming status on the server from the client.
  class IncomingCloseOnServer {
   public:
    IncomingCloseOnServer() = default;
    IncomingCloseOnServer(const IncomingCloseOnServer&) = delete;
    IncomingCloseOnServer& operator=(const IncomingCloseOnServer&) = delete;

    // Get the cancellation bit.
    bool was_cancelled() const { return cancelled_ != 0; }

    // Make a GRPC_OP_RECV_CLOSE_ON_SERVER op - intended for the framework, not
    // for tests.
    grpc_op MakeOp();

   private:
    int cancelled_;
  };

  // Build one batch. Returned from NewBatch (use that to instantiate this!)
  // Upon destruction of the BatchBuilder, the batch will be executed with any
  // added batches.
  class BatchBuilder {
   public:
    BatchBuilder(grpc_call* call, int tag) : call_(call), tag_(tag) {}
    ~BatchBuilder();

    BatchBuilder(const BatchBuilder&) = delete;
    BatchBuilder& operator=(const BatchBuilder&) = delete;
    BatchBuilder(BatchBuilder&&) noexcept = default;

    // Add a GRPC_OP_SEND_INITIAL_METADATA op.
    // Optionally specify flags, compression level.
    BatchBuilder& SendInitialMetadata(
        std::initializer_list<std::pair<absl::string_view, absl::string_view>>
            md,
        uint32_t flags = 0,
        absl::optional<grpc_compression_level> compression_level =
            absl::nullopt);

    // Add a GRPC_OP_SEND_MESSAGE op.
    BatchBuilder& SendMessage(Slice payload, uint32_t flags = 0);
    BatchBuilder& SendMessage(absl::string_view payload, uint32_t flags = 0) {
      return SendMessage(Slice::FromCopiedString(payload), flags);
    }

    // Add a GRPC_OP_SEND_CLOSE_FROM_CLIENT op.
    BatchBuilder& SendCloseFromClient();

    // Add a GRPC_OP_SEND_STATUS_FROM_SERVER op.
    BatchBuilder& SendStatusFromServer(
        grpc_status_code status, absl::string_view message,
        std::initializer_list<std::pair<absl::string_view, absl::string_view>>
            md);

    // Add a GRPC_OP_RECV_INITIAL_METADATA op.
    BatchBuilder& RecvInitialMetadata(IncomingMetadata& md) {
      ops_.emplace_back(md.MakeOp());
      return *this;
    }

    // Add a GRPC_OP_RECV_MESSAGE op.
    BatchBuilder& RecvMessage(IncomingMessage& msg) {
      ops_.emplace_back(msg.MakeOp());
      return *this;
    }

    // Add a GRPC_OP_RECV_STATUS_ON_CLIENT op.
    BatchBuilder& RecvStatusOnClient(IncomingStatusOnClient& status) {
      ops_.emplace_back(status.MakeOp());
      return *this;
    }

    // Add a GRPC_OP_RECV_CLOSE_ON_SERVER op.
    BatchBuilder& RecvCloseOnServer(IncomingCloseOnServer& close) {
      ops_.emplace_back(close.MakeOp());
      return *this;
    }

   private:
    // We need to track little bits of memory up until the batch is executed.
    // One Thing is one such block of memory.
    // We specialize it with SpecificThing to track a specific type of memory.
    // These get placed on things_ and deleted when the batch is executed.
    class Thing {
     public:
      virtual ~Thing() = default;
    };
    template <typename T>
    class SpecificThing final : public Thing {
     public:
      template <typename... Args>
      explicit SpecificThing(Args&&... args)
          : t_(std::forward<Args>(args)...) {}
      SpecificThing() = default;

      T& get() { return t_; }

     private:
      T t_;
    };

    // Make a thing of type T, and return a reference to it.
    template <typename T, typename... Args>
    T& Make(Args&&... args) {
      things_.emplace_back(new SpecificThing<T>(std::forward<Args>(args)...));
      return static_cast<SpecificThing<T>*>(things_.back().get())->get();
    }

    grpc_call* call_;
    const int tag_;
    std::vector<grpc_op> ops_;
    std::vector<std::unique_ptr<Thing>> things_;
  };

  // Wrapper around a grpc_call.
  // Instantiated by ClientCallBuilder via NewClientCall for client calls.
  // Wrapped by IncomingCall for server calls.
  class Call {
   public:
    explicit Call(grpc_call* call) : call_(call) {}
    Call(const Call&) = delete;
    Call& operator=(const Call&) = delete;
    Call(Call&& other) noexcept : call_(std::exchange(other.call_, nullptr)) {}
    ~Call() {
      if (call_ != nullptr) grpc_call_unref(call_);
    }
    // Construct a batch with a tag - upon destruction of the BatchBuilder the
    // operation will occur.
    BatchBuilder NewBatch(int tag) { return BatchBuilder(call_, tag); }
    // Cancel the call
    void Cancel() { grpc_call_cancel(call_, nullptr); }
    void CancelWithStatus(grpc_status_code status, const char* message) {
      grpc_call_cancel_with_status(call_, status, message, nullptr);
    }
    // Access the peer structure (returns a string that can be matched, etc) -
    // or nullopt if grpc_call_get_peer returns nullptr.
    absl::optional<std::string> GetPeer() {
      char* peer = grpc_call_get_peer(call_);
      if (peer == nullptr) return absl::nullopt;
      std::string result(peer);
      gpr_free(peer);
      return result;
    }

    // Set call credentials.
    // Takes ownership of creds.
    void SetCredentials(grpc_call_credentials* creds) {
      EXPECT_EQ(grpc_call_set_credentials(call_, creds), GRPC_CALL_OK);
      grpc_call_credentials_release(creds);
    }

    // Retrieve the auth context.
    std::unique_ptr<grpc_auth_context, void (*)(grpc_auth_context*)>
    GetAuthContext() {
      return std::unique_ptr<grpc_auth_context, void (*)(grpc_auth_context*)>(
          grpc_call_auth_context(call_), grpc_auth_context_release);
    }

    grpc_call** call_ptr() { return &call_; }
    grpc_call* c_call() const { return call_; }

   private:
    grpc_call* call_ = nullptr;
  };

  // Wrapper around a server call.
  class IncomingCall {
   public:
    IncomingCall(CoreEnd2endTest& test, int tag);
    IncomingCall(const IncomingCall&) = delete;
    IncomingCall& operator=(const IncomingCall&) = delete;
    IncomingCall(IncomingCall&&) noexcept = default;

    // Construct a batch with a tag - upon destruction of the BatchBuilder the
    // operation will occur. Must have received the call first!
    BatchBuilder NewBatch(int tag) { return impl_->call.NewBatch(tag); }
    void Cancel() { impl_->call.Cancel(); }

    // Return the method being called.
    std::string method() const {
      return std::string(StringViewFromSlice(impl_->call_details.method));
    }

    // Return the host being called.
    std::string host() const {
      return std::string(StringViewFromSlice(impl_->call_details.host));
    }

    // Return some initial metadata.
    absl::optional<std::string> GetInitialMetadata(absl::string_view key) const;

    // Return the peer address.
    absl::optional<std::string> GetPeer() { return impl_->call.GetPeer(); }

    // Return the auth context.
    std::unique_ptr<grpc_auth_context, void (*)(grpc_auth_context*)>
    GetAuthContext() {
      return impl_->call.GetAuthContext();
    }

    // Return the underlying C call object
    grpc_call* c_call() { return impl_->call.c_call(); }

    // Return the encodings accepted by the peer.
    BitSet<GRPC_COMPRESS_ALGORITHMS_COUNT> GetEncodingsAcceptedByPeer() {
      return BitSet<GRPC_COMPRESS_ALGORITHMS_COUNT>::FromInt(
          grpc_call_test_only_get_encodings_accepted_by_peer(c_call()));
    }

   private:
    struct Impl {
      Impl() {
        grpc_call_details_init(&call_details);
        grpc_metadata_array_init(&request_metadata);
      }
      ~Impl() {
        grpc_call_details_destroy(&call_details);
        grpc_metadata_array_destroy(&request_metadata);
      }
      Call call{nullptr};
      grpc_call_details call_details;
      grpc_metadata_array request_metadata;
    };
    std::unique_ptr<Impl> impl_;
  };

  // Begin construction of a client call.
  ClientCallBuilder NewClientCall(std::string method) {
    return ClientCallBuilder(*this, std::move(method));
  }
  ClientCallBuilder NewClientCall(RegisteredCall registered_method) {
    return ClientCallBuilder(*this, registered_method);
  }
  // Request a call on the server - notifies `tag` when complete.
  IncomingCall RequestCall(int tag) { return IncomingCall(*this, tag); }

  // Pull in CqVerifier types for ergonomics
  // TODO(ctiller): evaluate just dropping CqVerifier and folding it in here.
  using ExpectedResult = CqVerifier::ExpectedResult;
  using Maybe = CqVerifier::Maybe;
  using PerformAction = CqVerifier::PerformAction;
  using MaybePerformAction = CqVerifier::MaybePerformAction;
  using AnyStatus = CqVerifier::AnyStatus;
  // Expect a tag with some result.
  void Expect(int tag, ExpectedResult result, SourceLocation whence = {}) {
    expectations_++;
    cq_verifier().Expect(CqVerifier::tag(tag), result, whence);
  }
  // Step the system until expectations are met or until timeout is reached.
  // If there are no expectations logged, then step for 1 second and verify that
  // no events occur.
  void Step(absl::optional<Duration> timeout = absl::nullopt,
            SourceLocation whence = {}) {
    if (expectations_ == 0) {
      cq_verifier().VerifyEmpty(timeout.value_or(Duration::Seconds(1)), whence);
      return;
    }
    expectations_ = 0;
    cq_verifier().Verify(timeout.value_or(Duration::Seconds(10)), whence);
  }

  // Initialize the client.
  // If called, then InitServer must be called to create a server (otherwise one
  // will be provided).
  void InitClient(const ChannelArgs& args) {
    initialized_ = true;
    fixture().InitClient(args);
  }
  // Initialize the server.
  // If called, then InitClient must be called to create a client (otherwise one
  // will be provided).
  void InitServer(const ChannelArgs& args) {
    initialized_ = true;
    fixture().InitServer(args);
  }
  // Remove the client.
  void ShutdownAndDestroyClient() { fixture().ShutdownClient(); }
  // Shutdown the server; notify tag on completion.
  void ShutdownServerAndNotify(int tag) {
    grpc_server_shutdown_and_notify(fixture().server(), fixture().cq(),
                                    CqVerifier::tag(tag));
  }
  // Destroy the server.
  void DestroyServer() { fixture().DestroyServer(); }
  // Shutdown then destroy the server.
  void ShutdownAndDestroyServer() { fixture().ShutdownServer(); }
  // Cancel any calls on the server.
  void CancelAllCallsOnServer() {
    grpc_server_cancel_all_calls(fixture().server());
  }
  // Ping the server from the client
  void PingServerFromClient(int tag) {
    grpc_channel_ping(fixture().client(), fixture().cq(), CqVerifier::tag(tag),
                      nullptr);
  }
  // Register a call on the client, return its handle.
  RegisteredCall RegisterCallOnClient(const char* method, const char* host) {
    ForceInitialized();
    return RegisteredCall{
        grpc_channel_register_call(fixture().client(), method, host, nullptr)};
  }

  // Return the current connectivity state of the client.
  grpc_connectivity_state CheckConnectivityState(bool try_to_connect) {
    return grpc_channel_check_connectivity_state(fixture().client(),
                                                 try_to_connect);
  }

  // Watch the connectivity state of the client.
  void WatchConnectivityState(grpc_connectivity_state last_observed_state,
                              Duration deadline, int tag) {
    grpc_channel_watch_connectivity_state(
        fixture().client(), last_observed_state,
        grpc_timeout_milliseconds_to_deadline(deadline.millis()),
        fixture().cq(), CqVerifier::tag(tag));
  }

  // Return the client channel.
  grpc_channel* client() {
    ForceInitialized();
    return fixture().client();
  }

  // Return the server channel.
  grpc_server* server() {
    ForceInitialized();
    return fixture().server();
  }

  // Given a duration, return a timestamp that is that duration in the future -
  // with dilation according to test environment (eg sanitizers)
  Timestamp TimestampAfterDuration(Duration duration) {
    return Timestamp::FromTimespecRoundUp(
        grpc_timeout_milliseconds_to_deadline(duration.millis()));
  }

 private:
  void ForceInitialized();

  CoreTestFixture& fixture() {
    if (fixture_ == nullptr) {
      grpc_init();
      fixture_ = GetParam()->create_fixture(ChannelArgs(), ChannelArgs());
    }
    return *fixture_;
  }

  CqVerifier& cq_verifier() {
    if (cq_verifier_ == nullptr) {
      cq_verifier_ = absl::make_unique<CqVerifier>(
          fixture().cq(), CqVerifier::FailUsingGtestFail);
    }
    return *cq_verifier_;
  }

  std::unique_ptr<CoreTestFixture> fixture_;
  std::unique_ptr<CqVerifier> cq_verifier_;
  int expectations_ = 0;
  bool initialized_ = false;
};

// Define names for additional test suites.
// These make no changes to the actual class, but define new names to register
// tests against. Each new name gets a differing set of configurations in
// end2end_test_main.cc to customize the set of fixtures the tests run against.

// Test suite for tests that rely on a secure transport
class SecureEnd2endTest : public CoreEnd2endTest {};
// Test suite for tests that send rather large messages/metadata
class CoreLargeSendTest : public CoreEnd2endTest {};
// Test suite for tests that need a client channel
class CoreClientChannelTest : public CoreEnd2endTest {};
// Test suite for tests that require deadline handling
class CoreDeadlineTest : public CoreEnd2endTest {};
// Test suite for http2 tests that only work over a single hop (unproxyable)
class Http2SingleHopTest : public CoreEnd2endTest {};
// Test suite for tests that require retry features
class RetryTest : public CoreEnd2endTest {};
// Test suite for write buffering
class WriteBufferingTest : public CoreEnd2endTest {};
// Test suite for http2 tests
class Http2Test : public CoreEnd2endTest {};
// Test suite for http2 tests that require retry features
class RetryHttp2Test : public CoreEnd2endTest {};
// Test suite for tests that require resource quota
class ResourceQuotaTest : public CoreEnd2endTest {};
// Test suite for tests that require a transport that supports secure call
// credentials
class PerCallCredsTest : public CoreEnd2endTest {};
// Test suite for tests that require a transport that supports insecure call
// credentials
class PerCallCredsOnInsecureTest : public CoreEnd2endTest {};
// Test suite for tests that verify lack of logging in particular situations
class NoLoggingTest : public CoreEnd2endTest {};
// Test suite for tests that verify proxy authentication
class ProxyAuthTest : public CoreEnd2endTest {};

// A ConfigQuery queries a database a set of test configurations
// that match some criteria.
template <std::vector<CoreTestConfiguration> (*Configs)()>
class ConfigQuery {
 public:
  ConfigQuery() = default;
  ConfigQuery(const ConfigQuery&) = delete;
  ConfigQuery& operator=(const ConfigQuery&) = delete;
  // Enforce that the returned configurations have the given features.
  ConfigQuery& EnforceFeatures(uint32_t features) {
    enforce_features_ |= features;
    return *this;
  }
  // Envorce that the returned configurations do not have the given features.
  ConfigQuery& ExcludeFeatures(uint32_t features) {
    exclude_features_ |= features;
    return *this;
  }
  // Enforce that the returned configurations have the given name (regex).
  ConfigQuery& AllowName(const std::string& name) {
    allowed_names_.emplace_back(
        std::regex(name, std::regex_constants::ECMAScript));
    return *this;
  }
  // Enforce that the returned configurations do not have the given name
  // (regex).
  ConfigQuery& ExcludeName(const std::string& name) {
    excluded_names_.emplace_back(
        std::regex(name, std::regex_constants::ECMAScript));
    return *this;
  }

  auto Run() const {
    static NoDestruct<std::vector<CoreTestConfiguration>> kConfigs(Configs());
    std::vector<const CoreTestConfiguration*> out;
    for (const CoreTestConfiguration& config : *kConfigs) {
      if ((config.feature_mask & enforce_features_) == enforce_features_ &&
          (config.feature_mask & exclude_features_) == 0) {
        bool allowed = allowed_names_.empty();
        for (const std::regex& re : allowed_names_) {
          if (std::regex_match(config.name, re)) {
            allowed = true;
            break;
          }
        }
        for (const std::regex& re : excluded_names_) {
          if (std::regex_match(config.name, re)) {
            allowed = false;
            break;
          }
        }
        if (allowed) {
          out.push_back(&config);
        }
      }
    }
    return ::testing::ValuesIn(out);
  }

 private:
  uint32_t enforce_features_ = 0;
  uint32_t exclude_features_ = 0;
  std::vector<std::regex> allowed_names_;
  std::vector<std::regex> excluded_names_;
};

// Produce a nice name to print next to each test case for gtest.
inline const char* NameFromConfig(
    const ::testing::TestParamInfo<const CoreTestConfiguration*>& config) {
  return config.param->name;
}

// Instantiate all the test suites with the given set of configurations.
// Should only be called from inside namespace grpc_core.
#define INSTANTIATE_CORE_TEST_SUITES(configs)                                  \
  INSTANTIATE_TEST_SUITE_P(CoreEnd2endTests, CoreEnd2endTest,                  \
                           ConfigQuery<configs>().Run(), NameFromConfig);      \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      SecureEnd2endTests, SecureEnd2endTest,                                   \
      ConfigQuery<configs>().EnforceFeatures(FEATURE_MASK_IS_SECURE).Run(),    \
      NameFromConfig);                                                         \
  INSTANTIATE_TEST_SUITE_P(CoreLargeSendTests, CoreLargeSendTest,              \
                           ConfigQuery<configs>()                              \
                               .ExcludeFeatures(FEATURE_MASK_1BYTE_AT_A_TIME | \
                                                FEATURE_MASK_ENABLES_TRACES)   \
                               .Run(),                                         \
                           NameFromConfig);                                    \
                                                                               \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      CoreDeadlineTests, CoreDeadlineTest,                                     \
      ConfigQuery<configs>().ExcludeFeatures(FEATURE_MASK_IS_MINSTACK).Run(),  \
      NameFromConfig);                                                         \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      CoreClientChannelTests, CoreClientChannelTest,                           \
      ConfigQuery<configs>()                                                   \
          .EnforceFeatures(FEATURE_MASK_SUPPORTS_CLIENT_CHANNEL)               \
          .Run(),                                                              \
      NameFromConfig);                                                         \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      Http2SingleHopTests, Http2SingleHopTest,                                 \
      ConfigQuery<configs>()                                                   \
          .EnforceFeatures(FEATURE_MASK_IS_HTTP2)                              \
          .ExcludeFeatures(FEATURE_MASK_SUPPORTS_REQUEST_PROXYING |            \
                           FEATURE_MASK_ENABLES_TRACES)                        \
          .Run(),                                                              \
      NameFromConfig);                                                         \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      RetryTests, RetryTest,                                                   \
      ConfigQuery<configs>()                                                   \
          .EnforceFeatures(FEATURE_MASK_SUPPORTS_CLIENT_CHANNEL)               \
          .ExcludeFeatures(FEATURE_MASK_DOES_NOT_SUPPORT_RETRY)                \
          .Run(),                                                              \
      NameFromConfig);                                                         \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      WriteBufferingTests, WriteBufferingTest,                                 \
      ConfigQuery<configs>()                                                   \
          .ExcludeFeatures(FEATURE_MASK_DOES_NOT_SUPPORT_WRITE_BUFFERING)      \
          .Run(),                                                              \
      NameFromConfig);                                                         \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      Http2Tests, Http2Test,                                                   \
      ConfigQuery<configs>().EnforceFeatures(FEATURE_MASK_IS_HTTP2).Run(),     \
      NameFromConfig);                                                         \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      RetryHttp2Tests, RetryHttp2Test,                                         \
      ConfigQuery<configs>()                                                   \
          .EnforceFeatures(FEATURE_MASK_IS_HTTP2 |                             \
                           FEATURE_MASK_SUPPORTS_CLIENT_CHANNEL)               \
          .ExcludeFeatures(FEATURE_MASK_DOES_NOT_SUPPORT_RETRY |               \
                           FEATURE_MASK_SUPPORTS_REQUEST_PROXYING)             \
          .Run(),                                                              \
      NameFromConfig);                                                         \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      ResourceQuotaTests, ResourceQuotaTest,                                   \
      ConfigQuery<configs>()                                                   \
          .ExcludeFeatures(FEATURE_MASK_SUPPORTS_REQUEST_PROXYING |            \
                           FEATURE_MASK_1BYTE_AT_A_TIME)                       \
          .ExcludeName("Chttp2.*Uds.*")                                        \
          .ExcludeName("Chttp2HttpProxy")                                      \
          .Run(),                                                              \
      NameFromConfig);                                                         \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      PerCallCredsTests, PerCallCredsTest,                                     \
      ConfigQuery<configs>()                                                   \
          .EnforceFeatures(FEATURE_MASK_SUPPORTS_PER_CALL_CREDENTIALS)         \
          .Run(),                                                              \
      NameFromConfig);                                                         \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      PerCallCredsOnInsecureTests, PerCallCredsOnInsecureTest,                 \
      ConfigQuery<configs>()                                                   \
          .EnforceFeatures(                                                    \
              FEATURE_MASK_SUPPORTS_PER_CALL_CREDENTIALS_LEVEL_INSECURE)       \
          .Run(),                                                              \
      NameFromConfig);                                                         \
  INSTANTIATE_TEST_SUITE_P(NoLoggingTests, NoLoggingTest,                      \
                           ConfigQuery<configs>()                              \
                               .ExcludeFeatures(FEATURE_MASK_ENABLES_TRACES)   \
                               .Run(),                                         \
                           NameFromConfig);                                    \
  INSTANTIATE_TEST_SUITE_P(                                                    \
      ProxyAuthTests, ProxyAuthTest,                                           \
      ConfigQuery<configs>().AllowName("Chttp2HttpProxy").Run(),               \
      NameFromConfig);

}  // namespace grpc_core

// If this test fixture is being run under minstack, skip the test.
#define SKIP_IF_MINSTACK()                                 \
  if (GetParam()->feature_mask & FEATURE_MASK_IS_MINSTACK) \
  GTEST_SKIP() << "Skipping test for minstack"

#endif  // GRPC_TEST_CORE_END2END_END2END_TESTS_H
