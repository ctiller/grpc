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
  SpawnTestSeq(
      initiator, "initiator",
      [&]() {
        auto md = Arena::MakePooled<ClientMetadata>(GetContext<Arena>());
        md->Set(HttpPathMetadata(), Slice::FromExternalString("/foo/bar"));
        return initiator.PushClientInitialMetadata(std::move(md));
      },
      [&](StatusFlag status) mutable {
        EXPECT_TRUE(status.ok());
        initiator.FinishSends();
        return initiator.PullServerInitialMetadata();
      },
      [&](ValueOrFailure<ServerMetadataHandle> md) {
        EXPECT_TRUE(md.ok());
        EXPECT_EQ(*md.value()->get_pointer(ContentTypeMetadata()),
                  ContentTypeMetadata::kApplicationGrpc);
        return initiator.PullServerTrailingMetadata();
      },
      [&](ValueOrFailure<ServerMetadataHandle> md) {
        EXPECT_TRUE(md.ok());
        EXPECT_EQ(*md.value()->get_pointer(GrpcStatusMetadata()),
                  GRPC_STATUS_UNIMPLEMENTED);
        return Empty{};
      });
  auto handler = TickUntilServerCall();
  SpawnTestSeq(
      handler, "handler", [&] { return handler.PullClientInitialMetadata(); },
      [&](ValueOrFailure<ServerMetadataHandle> md) {
        EXPECT_TRUE(md.ok());
        EXPECT_EQ(md.value()->get_pointer(HttpPathMetadata())->as_string_view(),
                  "/foo/bar");
        return handler.PullMessage();
      },
      [&](NextResult<MessageHandle> msg) {
        EXPECT_FALSE(msg.has_value());
        auto md = Arena::MakePooled<ServerMetadata>(GetContext<Arena>());
        md->Set(ContentTypeMetadata(), ContentTypeMetadata::kApplicationGrpc);
        return handler.PushServerInitialMetadata(std::move(md));
      },
      [&](StatusFlag result) mutable {
        EXPECT_TRUE(result.ok());
        auto md = Arena::MakePooled<ServerMetadata>(GetContext<Arena>());
        md->Set(GrpcStatusMetadata(), GRPC_STATUS_UNIMPLEMENTED);
        return handler.PushServerTrailingMetadata(std::move(md));
      },
      [&](StatusFlag result) mutable {
        EXPECT_TRUE(result.ok());
        return Empty{};
      });
  WaitForAllPendingWork();
}

TRANSPORT_TEST(MetadataOnlyRequestServerAbortsAfterInitialMetadata) {
  SetServerAcceptor();
  auto initiator = CreateCall();
  SpawnTestSeq(
      initiator, "initiator",
      [&]() {
        auto md = Arena::MakePooled<ClientMetadata>(GetContext<Arena>());
        md->Set(HttpPathMetadata(), Slice::FromExternalString("/foo/bar"));
        return initiator.PushClientInitialMetadata(std::move(md));
      },
      [&](StatusFlag status) mutable {
        EXPECT_TRUE(status.ok());
        // We don't close the sending stream here.
        return initiator.PullServerInitialMetadata();
      },
      [&](ValueOrFailure<ServerMetadataHandle> md) {
        EXPECT_TRUE(md.ok());
        EXPECT_EQ(*md.value()->get_pointer(ContentTypeMetadata()),
                  ContentTypeMetadata::kApplicationGrpc);
        return initiator.PullServerTrailingMetadata();
      },
      [&](ValueOrFailure<ServerMetadataHandle> md) {
        EXPECT_TRUE(md.ok());
        EXPECT_EQ(*md.value()->get_pointer(GrpcStatusMetadata()),
                  GRPC_STATUS_UNIMPLEMENTED);
        return Empty{};
      });
  auto handler = TickUntilServerCall();
  SpawnTestSeq(
      handler, "handler", [&] { return handler.PullClientInitialMetadata(); },
      [&](ValueOrFailure<ServerMetadataHandle> got_md) {
        EXPECT_TRUE(got_md.ok());
        EXPECT_EQ(
            got_md.value()->get_pointer(HttpPathMetadata())->as_string_view(),
            "/foo/bar");
        // Don't wait for end of stream for client->server messages, just
        // publish initial then trailing metadata.
        auto md = Arena::MakePooled<ServerMetadata>(GetContext<Arena>());
        md->Set(ContentTypeMetadata(), ContentTypeMetadata::kApplicationGrpc);
        return handler.PushServerInitialMetadata(std::move(md));
      },
      [&](StatusFlag result) mutable {
        EXPECT_TRUE(result.ok());
        auto md = Arena::MakePooled<ServerMetadata>(GetContext<Arena>());
        md->Set(GrpcStatusMetadata(), GRPC_STATUS_UNIMPLEMENTED);
        return handler.PushServerTrailingMetadata(std::move(md));
      },
      [&](StatusFlag result) mutable {
        EXPECT_TRUE(result.ok());
        return Empty{};
      });
  WaitForAllPendingWork();
}

TRANSPORT_TEST(MetadataOnlyRequestServerAbortsImmediately) {
  SetServerAcceptor();
  auto initiator = CreateCall();
  SpawnTestSeq(
      initiator, "initiator",
      [&]() {
        auto md = Arena::MakePooled<ClientMetadata>(GetContext<Arena>());
        md->Set(HttpPathMetadata(), Slice::FromExternalString("/foo/bar"));
        return initiator.PushClientInitialMetadata(std::move(md));
      },
      [&](StatusFlag status) mutable {
        EXPECT_TRUE(status.ok());
        // We don't close the sending stream here.
        return initiator.PullServerInitialMetadata();
      },
      [&](ValueOrFailure<ServerMetadataHandle> md) {
        EXPECT_FALSE(md.ok());
        return initiator.PullServerTrailingMetadata();
      },
      [&](ValueOrFailure<ServerMetadataHandle> md) {
        EXPECT_TRUE(md.ok());
        EXPECT_EQ(*md.value()->get_pointer(GrpcStatusMetadata()),
                  GRPC_STATUS_UNIMPLEMENTED);
        return Empty{};
      });
  auto handler = TickUntilServerCall();
  SpawnTestSeq(
      handler, "handler", [&] { return handler.PullClientInitialMetadata(); },
      [&](ValueOrFailure<ServerMetadataHandle> got_md) {
        EXPECT_TRUE(got_md.ok());
        EXPECT_EQ(
            got_md.value()->get_pointer(HttpPathMetadata())->as_string_view(),
            "/foo/bar");
        // Don't wait for end of stream for client->server messages, just
        // and don't send initial metadata - just trailing metadata.
        auto md = Arena::MakePooled<ServerMetadata>(GetContext<Arena>());
        md->Set(GrpcStatusMetadata(), GRPC_STATUS_UNIMPLEMENTED);
        return handler.PushServerTrailingMetadata(std::move(md));
      },
      [&](StatusFlag result) mutable {
        EXPECT_TRUE(result.ok());
        return Empty{};
      });
  WaitForAllPendingWork();
}

TRANSPORT_TEST(CanCreateCallThenAbandonIt) {
  SetServerAcceptor();
  auto initiator = CreateCall();
  SpawnTestSeq(
      initiator, "start-call",
      [&]() {
        auto md = Arena::MakePooled<ClientMetadata>(GetContext<Arena>());
        md->Set(HttpPathMetadata(), Slice::FromExternalString("/foo/bar"));
        return initiator.PushClientInitialMetadata(std::move(md));
      },
      [&](StatusFlag status) mutable {
        EXPECT_TRUE(status.ok());
        return Empty{};
      });
  auto handler = TickUntilServerCall();
  SpawnTestSeq(initiator, "end-call", [&]() {
    initiator.Cancel();
    return Empty{};
  });
  WaitForAllPendingWork();
}

}  // namespace grpc_core
