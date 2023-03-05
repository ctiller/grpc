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

#ifndef GRPC_TEST_CORE_END2END_FIXTURES_SECURE_FIXTURE_H
#define GRPC_TEST_CORE_END2END_FIXTURES_SECURE_FIXTURE_H

#include <grpc/grpc_security.h>

#include "src/core/lib/gprpp/host_port.h"
#include "test/core/end2end/end2end_tests.h"
#include "test/core/util/port.h"

class SecureFixture : public CoreTestFixture {
 public:
  explicit SecureFixture(std::string localaddr = grpc_core::JoinHostPort(
                             "localhost", grpc_pick_unused_port_or_die()))
      : localaddr_(std::move(localaddr)) {}

 private:
  virtual grpc_channel_credentials* MakeClientCreds(
      const grpc_core::ChannelArgs& args) = 0;
  virtual grpc_server_credentials* MakeServerCreds(
      const grpc_core::ChannelArgs& args) = 0;
  virtual grpc_core::ChannelArgs MutateClientArgs(grpc_core::ChannelArgs args) {
    return args;
  }
  virtual grpc_core::ChannelArgs MutateServerArgs(grpc_core::ChannelArgs args) {
    return args;
  }
  grpc_server* MakeServer(const grpc_core::ChannelArgs& in_args) override {
    auto args = MutateServerArgs(in_args);
    auto* creds = MakeServerCreds(args);
    auto* server = grpc_server_create(args.ToC().get(), nullptr);
    grpc_server_register_completion_queue(server, cq(), nullptr);
    GPR_ASSERT(grpc_server_add_http2_port(server, localaddr_.c_str(), creds));
    grpc_server_credentials_release(creds);
    grpc_server_start(server);
    return server;
  }
  grpc_channel* MakeClient(const grpc_core::ChannelArgs& in_args) override {
    auto args = MutateClientArgs(in_args);
    auto* creds = MakeClientCreds(args);
    auto* client =
        grpc_channel_create(localaddr_.c_str(), creds, args.ToC().get());
    GPR_ASSERT(client != nullptr);
    grpc_channel_credentials_release(creds);
    return client;
  }

  std::string localaddr_;
};

class InsecureFixture : public SecureFixture {
 public:
  using SecureFixture::SecureFixture;

 private:
  grpc_channel_credentials* MakeClientCreds(
      const grpc_core::ChannelArgs& args) override {
    return grpc_insecure_credentials_create();
  }
  grpc_server_credentials* MakeServerCreds(
      const grpc_core::ChannelArgs& args) override {
    return grpc_insecure_server_credentials_create();
  }
};

#endif  // GRPC_TEST_CORE_END2END_FIXTURES_SECURE_FIXTURE_H
