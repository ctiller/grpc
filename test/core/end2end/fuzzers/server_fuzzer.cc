// Copyright 2016 gRPC authors.
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

#include <grpc/grpc.h>
#include <grpc/grpc_security.h>
#include <grpc/slice.h>

#include <optional>
#include <string>

#include "absl/log/check.h"
#include "src/core/config/core_configuration.h"
#include "src/core/ext/transport/chaotic_good/server/chaotic_good_server.h"
#include "src/core/lib/event_engine/tcp_socket_utils.h"
#include "src/core/lib/experiments/config.h"
#include "src/core/lib/iomgr/exec_ctx.h"
#include "src/core/util/env.h"
#include "test/core/end2end/fuzzers/api_fuzzer.pb.h"
#include "test/core/end2end/fuzzers/fuzzer_input.pb.h"
#include "test/core/end2end/fuzzers/fuzzing_common.h"
#include "test/core/end2end/fuzzers/network_input.h"
#include "test/core/test_util/fuzz_config_vars.h"
#include "test/core/test_util/test_config.h"
#include "fuzztest/fuzztest.h"
#include "src/core/lib/security/credentials/fake/fake_credentials.h"

bool squelch = true;
bool leak_check = true;

namespace grpc_core {
namespace testing {

class ServerFuzzer final : public BasicFuzzer {
 public:
  explicit ServerFuzzer(
      const fuzzer_input::Msg& msg,
      absl::FunctionRef<void(grpc_server*, int, const ChannelArgs&)>
          server_setup)
      : BasicFuzzer(msg.event_engine_actions()) {
    ExecCtx exec_ctx;
    grpc_server_register_completion_queue(server_, cq(), nullptr);
    // TODO(ctiller): add more registered methods (one for POST, one for PUT)
    grpc_server_register_method(server_, "/reg", nullptr, {}, 0);
    server_setup(
        server_, 1234,
        CoreConfiguration::Get()
            .channel_args_preconditioning()
            .PreconditionChannelArgs(
                CreateChannelArgsFromFuzzingConfiguration(
                    msg.channel_args(), FuzzingEnvironment{resource_quota()})
                    .ToC()
                    .get()));
    grpc_server_start(server_);
    for (const auto& input : msg.network_input()) {
      UpdateMinimumRunTime(ScheduleConnection(
          input, engine().get(), FuzzingEnvironment{resource_quota()}, 1234));
    }
  }

  ~ServerFuzzer() { CHECK_EQ(server_, nullptr); }

 private:
  Result CreateChannel(
      const api_fuzzer::CreateChannel& /* create_channel */) override {
    return Result::kFailed;
  }
  Result CreateServer(
      const api_fuzzer::CreateServer& /* create_server */) override {
    return Result::kFailed;
  }
  void DestroyServer() override {
    grpc_server_destroy(server_);
    server_ = nullptr;
  }
  void DestroyChannel() override {}

  grpc_server* server() override { return server_; }
  grpc_channel* channel() override { return nullptr; }

  grpc_server* server_ = grpc_server_create(nullptr, nullptr);
};

void RunServerFuzzer(
    const fuzzer_input::Msg& msg,
    absl::FunctionRef<void(grpc_server*, int, const ChannelArgs&)>
        server_setup) {
  if (squelch && !GetEnv("GRPC_TRACE_FUZZER").has_value()) {
    grpc_disable_all_absl_logs();
  }
  static const int once = []() {
    ForceEnableExperiment("event_engine_client", true);
    ForceEnableExperiment("event_engine_listener", true);
    return 42;
  }();
  CHECK_EQ(once, 42);  // avoid unused variable warning
  ApplyFuzzConfigVars(msg.config_vars());
  TestOnlyReloadExperimentsFromConfigVariables();
  testing::ServerFuzzer(msg, server_setup).Run(msg.api_actions());
}

void ChaoticGood(fuzzer_input::Msg msg) {
  RunServerFuzzer(
      msg, [](grpc_server* server, int port_num,
              const ChannelArgs& channel_args) {
        ExecCtx exec_ctx;
        auto* listener = new chaotic_good::ChaoticGoodServerListener(
            Server::FromC(server), channel_args,
            [next = uint64_t(0)]() mutable {
              return absl::StrCat(absl::Hex(next++));
            });
        auto port = listener->Bind(
            grpc_event_engine::experimental::URIToResolvedAddress(
                absl::StrCat("ipv4:0.0.0.0:", port_num))
                .value());
        CHECK_OK(port);
        CHECK_EQ(port.value(), port_num);
        Server::FromC(server)->AddListener(
            OrphanablePtr<
                chaotic_good::ChaoticGoodServerListener>(listener));
      });
}
FUZZ_TEST(ServerFuzzers, ChaoticGood);

void Chttp2(fuzzer_input::Msg msg) {
 RunServerFuzzer(msg, [](grpc_server* server, int port_num,
                                     const ChannelArgs&) {
    auto* creds = grpc_insecure_server_credentials_create();
    grpc_server_add_http2_port(
        server, absl::StrCat("0.0.0.0:", port_num).c_str(), creds);
    grpc_server_credentials_release(creds);
  });
}
FUZZ_TEST(ServerFuzzers, Chttp2);

void Chttp2FakeSec(fuzzer_input::Msg msg) {
  RunServerFuzzer(msg, [](grpc_server* server, int port_num,
                                     const ChannelArgs&) {
    auto* creds = grpc_fake_transport_security_server_credentials_create();
    grpc_server_add_http2_port(
        server, absl::StrCat("0.0.0.0:", port_num).c_str(), creds);
    grpc_server_credentials_release(creds);
  });
}
FUZZ_TEST(ServerFuzzers, Chttp2FakeSec);

}  // namespace testing
}  // namespace grpc_core
