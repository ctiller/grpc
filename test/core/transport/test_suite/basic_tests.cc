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

TRANSPORT_TEST(MetadataOnlyRequest) {
  SetServerAcceptor();
  auto initiator = CreateCall();
  Event client_done;
  initiator.SpawnInfallible("initiator", [&]() {
    auto md = Arena::MakePooled<ClientMetadata>(GetContext<Arena>());
    md->Set(HttpPathMetadata(), Slice::FromExternalString("/foo/bar"));
    return Seq(
        initiator.PushClientInitialMetadata(std::move(md)),
        [initiator](StatusFlag status) mutable {
          EXPECT_TRUE(status.ok());
          initiator.FinishSends();
          return Empty{};
        },
        [client_done]() mutable {
          client_done.Set();
          return Empty{};
        });
  });
  auto handler = TickUntilServerCall();
  Event server_done;
  handler.SpawnInfallible("handler", [handler, server_done]() mutable {
    return Seq(
        handler.PullClientInitialMetadata(),
        [](ValueOrFailure<ServerMetadataHandle> md) {
          EXPECT_TRUE(md.ok());
          EXPECT_EQ(
              md.value()->get_pointer(HttpPathMetadata())->as_string_view(),
              "/foo/bar");
          return Empty{};
        },
        handler.PullMessage(),
        [](NextResult<MessageHandle> msg) {
          EXPECT_FALSE(msg.has_value());
          return Empty{};
        },
        [handler]() mutable {
          auto md = Arena::MakePooled<ServerMetadata>(GetContext<Arena>());
          md->Set(ContentTypeMetadata(), ContentTypeMetadata::kApplicationGrpc);
          return handler.PushServerInitialMetadata(std::move(md));
        },
        [handler]() mutable {
          auto md = Arena::MakePooled<ServerMetadata>(GetContext<Arena>());
          md->Set(GrpcStatusMetadata(), GRPC_STATUS_UNIMPLEMENTED);
          return handler.PushServerTrailingMetadata(std::move(md));
        },
        [server_done]() mutable {
          server_done.Set();
          return Empty{};
        });
  });
  TickUntilEvents({client_done, server_done});
}

TRANSPORT_TEST(CanCreateCallThenAbandonIt) {
  SetServerAcceptor();
  auto initiator = CreateCall();
  initiator.SpawnGuarded("start-call", [&]() {
    auto md = Arena::MakePooled<ClientMetadata>(GetContext<Arena>());
    md->Set(HttpPathMetadata(), Slice::FromExternalString("/foo/bar"));
    return initiator.PushClientInitialMetadata(std::move(md));
  });
  auto handler = TickUntilServerCall();
  initiator.SpawnInfallible("end-call", [&]() {
    initiator.Cancel();
    return Empty{};
  });
}

}  // namespace grpc_core
