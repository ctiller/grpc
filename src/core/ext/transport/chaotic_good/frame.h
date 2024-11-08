// Copyright 2022 gRPC authors.
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

#ifndef GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_FRAME_H
#define GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_FRAME_H

#include <grpc/support/port_platform.h>

#include <cstdint>
#include <limits>
#include <string>

#include "absl/random/bit_gen_ref.h"
#include "absl/status/status.h"
#include "absl/types/variant.h"
#include "src/core/ext/transport/chaotic_good/chaotic_good_frame.pb.h"
#include "src/core/ext/transport/chaotic_good/frame_header.h"
#include "src/core/lib/resource_quota/arena.h"
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/transport/message.h"
#include "src/core/lib/transport/metadata.h"
#include "src/core/util/match.h"

namespace grpc_core {
namespace chaotic_good {

absl::Status ReadProto(SliceBuffer payload, google::protobuf::MessageLite& msg);
void WriteProto(const google::protobuf::MessageLite& msg, SliceBuffer& output);
uint32_t ProtoPayloadSize(const google::protobuf::MessageLite& msg);

struct BufferPair {
  SliceBuffer control;
  SliceBuffer data;
};

class FrameInterface {
 public:
  virtual absl::Status Deserialize(const FrameHeader& header,
                                   SliceBuffer payload) = 0;
  virtual FrameHeader MakeHeader() const = 0;
  virtual void SerializePayload(SliceBuffer& payload) const = 0;
  virtual std::string ToString() const = 0;

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const FrameInterface& frame) {
    sink.Append(frame.ToString());
  }

 protected:
  ~FrameInterface() = default;
};

inline std::ostream& operator<<(std::ostream& os, const FrameInterface& frame) {
  return os << frame.ToString();
}

chaotic_good_frame::ClientMetadata ClientMetadataProtoFromGrpc(
    const ClientMetadata& md);
absl::StatusOr<ClientMetadataHandle> ClientMetadataGrpcFromProto(
    chaotic_good_frame::ClientMetadata& metadata);
chaotic_good_frame::ServerMetadata ServerMetadataProtoFromGrpc(
    const ServerMetadata& md);
absl::StatusOr<ServerMetadataHandle> ServerMetadataGrpcFromProto(
    chaotic_good_frame::ServerMetadata& metadata);

template <FrameType type, typename T>
struct ProtoFrame final : public FrameInterface {
  absl::Status Deserialize(const FrameHeader& header,
                           SliceBuffer payload) override {
    CHECK_EQ(header.type, type);
    if (header.stream_id != 0) {
      return absl::InternalError(
          absl::StrCat("Expected stream id 0 on ", header.ToString()));
    }
    return ReadProto(std::move(payload), body);
  }
  FrameHeader MakeHeader() const override {
    auto length = body.ByteSizeLong();
    CHECK_LE(length, std::numeric_limits<uint32_t>::max());
    return FrameHeader{type, 0, 0, static_cast<uint32_t>(length)};
  }
  void SerializePayload(SliceBuffer& payload) const override {
    WriteProto(body, payload);
  }
  std::string ToString() const override {
    return absl::StrCat(FrameTypeString(type), ":", body.ShortDebugString());
  }

  T body;
};

template <FrameType type, typename T>
struct ProtoStreamFrame final : public FrameInterface {
  absl::Status Deserialize(const FrameHeader& header,
                           SliceBuffer payload) override {
    CHECK_EQ(header.type, type);
    if (header.stream_id == 0) {
      return absl::InternalError(
          absl::StrCat("Expected non-zero stream id on ", header.ToString()));
    }
    stream_id = header.stream_id;
    return ReadProto(std::move(payload), body);
  }
  FrameHeader MakeHeader() const override {
    auto length = body.ByteSizeLong();
    CHECK_LE(length, std::numeric_limits<uint32_t>::max());
    return FrameHeader{type, 0, stream_id, static_cast<uint32_t>(length)};
  }
  void SerializePayload(SliceBuffer& payload) const override {
    WriteProto(body, payload);
  }
  std::string ToString() const override {
    return absl::StrCat(FrameTypeString(type), "@", stream_id, ": ",
                        body.ShortDebugString());
  }

  uint32_t stream_id;
  T body;
};

template <FrameType type>
struct EmptyStreamFrame final : public FrameInterface {
  absl::Status Deserialize(const FrameHeader& header,
                           SliceBuffer payload) override {
    CHECK_EQ(header.type, type);
    if (header.stream_id == 0) {
      return absl::InternalError(
          absl::StrCat("Expected non-zero stream id on ", header.ToString()));
    }
    if (payload.Length() != 0) {
      return absl::InternalError(
          absl::StrCat("Unexpected payload for ", header.ToString()));
    }
    stream_id = header.stream_id;
    return absl::OkStatus();
  }
  FrameHeader MakeHeader() const override {
    return FrameHeader{type, 0, stream_id, 0};
  }
  void SerializePayload(SliceBuffer& payload) const override {}
  std::string ToString() const override {
    return absl::StrCat(FrameTypeString(type), "@", stream_id);
  }

  uint32_t stream_id;
};

using SettingsFrame =
    ProtoFrame<FrameType::kSettings, chaotic_good_frame::Settings>;
using ClientInitialMetadataFrame =
    ProtoStreamFrame<FrameType::kClientInitialMetadata,
                     chaotic_good_frame::ClientMetadata>;
using BeginMessageFrame = ProtoStreamFrame<FrameType::kBeginMessage,
                                           chaotic_good_frame::BeginMessage>;
using ServerInitialMetadataFrame =
    ProtoStreamFrame<FrameType::kServerInitialMetadata,
                     chaotic_good_frame::ServerMetadata>;
using ServerTrailingMetadataFrame =
    ProtoStreamFrame<FrameType::kServerTrailingMetadata,
                     chaotic_good_frame::ServerMetadata>;
using GoAwayFrame = ProtoFrame<FrameType::kGoAway, chaotic_good_frame::GoAway>;
using SolicitRequestsFrame = ProtoFrame<FrameType::kSolicitRequests,
                                        chaotic_good_frame::SolicitRequests>;
using StreamWindowUpdateFrame =
    ProtoStreamFrame<FrameType::kStreamWindowUpdate,
                     chaotic_good_frame::StreamWindowUpdate>;
using StreamInitialWindowUpdateFrame =
    ProtoFrame<FrameType::kStreamInitialWindowUpdate,
               chaotic_good_frame::StreamInitialWindowUpdate>;
using TransportWindowUpdateFrame =
    ProtoFrame<FrameType::kTransportWindowUpdate,
               chaotic_good_frame::TransportWindowUpdate>;
using ClientEndOfStream = EmptyStreamFrame<FrameType::kClientEndOfStream>;
using CancelFrame = EmptyStreamFrame<FrameType::kCancel>;

struct MessageFrame final : public FrameInterface {
  absl::Status Deserialize(const FrameHeader& header,
                           SliceBuffer payload) override;
  FrameHeader MakeHeader() const override;
  void SerializePayload(SliceBuffer& payload) const override;
  std::string ToString() const override;

  uint32_t stream_id;
  MessageHandle message;
};

struct MessageChunkFrame final : public FrameInterface {
  absl::Status Deserialize(const FrameHeader& header,
                           SliceBuffer payload) override;
  FrameHeader MakeHeader() const override;
  void SerializePayload(SliceBuffer& payload) const override;
  std::string ToString() const override;

  uint32_t stream_id;
  SliceBuffer payload;
};

using ClientFrame =
    absl::variant<StreamWindowUpdateFrame, StreamInitialWindowUpdateFrame,
                  TransportWindowUpdateFrame, ClientInitialMetadataFrame,
                  MessageFrame, BeginMessageFrame, MessageChunkFrame,
                  ClientEndOfStream, CancelFrame>;
using ServerFrame =
    absl::variant<GoAwayFrame, SolicitRequestsFrame, StreamWindowUpdateFrame,
                  StreamInitialWindowUpdateFrame, TransportWindowUpdateFrame,
                  ServerInitialMetadataFrame, MessageFrame, BeginMessageFrame,
                  MessageChunkFrame, ServerTrailingMetadataFrame>;

inline FrameInterface& GetFrameInterface(ClientFrame& frame) {
  return MatchMutable(
      &frame,
      [](StreamWindowUpdateFrame* frame) -> FrameInterface& { return *frame; },
      [](StreamInitialWindowUpdateFrame* frame) -> FrameInterface& {
        return *frame;
      },
      [](TransportWindowUpdateFrame* frame) -> FrameInterface& {
        return *frame;
      },
      [](ClientInitialMetadataFrame* frame) -> FrameInterface& {
        return *frame;
      },
      [](MessageFrame* frame) -> FrameInterface& { return *frame; },
      [](BeginMessageFrame* frame) -> FrameInterface& { return *frame; },
      [](MessageChunkFrame* frame) -> FrameInterface& { return *frame; },
      [](ClientEndOfStream* frame) -> FrameInterface& { return *frame; },
      [](CancelFrame* frame) -> FrameInterface& { return *frame; });
}

inline FrameInterface& GetFrameInterface(ServerFrame& frame) {
  return MatchMutable(
      &frame, [](GoAwayFrame* frame) -> FrameInterface& { return *frame; },
      [](SolicitRequestsFrame* frame) -> FrameInterface& { return *frame; },
      [](StreamWindowUpdateFrame* frame) -> FrameInterface& { return *frame; },
      [](StreamInitialWindowUpdateFrame* frame) -> FrameInterface& {
        return *frame;
      },
      [](TransportWindowUpdateFrame* frame) -> FrameInterface& {
        return *frame;
      },
      [](ServerInitialMetadataFrame* frame) -> FrameInterface& {
        return *frame;
      },
      [](MessageFrame* frame) -> FrameInterface& { return *frame; },
      [](BeginMessageFrame* frame) -> FrameInterface& { return *frame; },
      [](MessageChunkFrame* frame) -> FrameInterface& { return *frame; },
      [](ServerTrailingMetadataFrame* frame) -> FrameInterface& {
        return *frame;
      });
}

}  // namespace chaotic_good
}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_FRAME_H
