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

#include <cstdint>
#include <string>

#include "absl/random/bit_gen_ref.h"
#include "absl/status/status.h"
#include "absl/types/variant.h"

#include <grpc/support/port_platform.h>

#include "src/core/ext/transport/chaotic_good/frame_header.h"
#include "src/core/ext/transport/chttp2/transport/hpack_encoder.h"
#include "src/core/ext/transport/chttp2/transport/hpack_parser.h"
#include "src/core/lib/gprpp/match.h"
#include "src/core/lib/resource_quota/arena.h"
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/transport/metadata_batch.h"

namespace grpc_core {
namespace chaotic_good {

struct BufferPair {
  SliceBuffer control;
  SliceBuffer data;
};

class FrameInterface {
 public:
  virtual absl::Status Deserialize(HPackParser* parser,
                                   const FrameHeader& header,
                                   absl::BitGenRef bitsrc,
                                   SliceBuffer payload) = 0;
  virtual void Serialize(HPackCompressor* encoder, bool& saw_encoding_errors,
                         BufferPair* out) const = 0;
  virtual std::string ToString() const = 0;

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const FrameInterface& frame) {
    sink.Append(frame.ToString());
  }

 protected:
  static bool EqVal(const grpc_metadata_batch& a,
                    const grpc_metadata_batch& b) {
    return a.DebugString() == b.DebugString();
  }
  template <typename T>
  static bool EqHdl(const Arena::PoolPtr<T>& a, const Arena::PoolPtr<T>& b) {
    if (a == nullptr && b == nullptr) return true;
    if (a == nullptr || b == nullptr) return false;
    return EqVal(*a, *b);
  }
  ~FrameInterface() = default;
};

inline std::ostream& operator<<(std::ostream& os, const FrameInterface& frame) {
  return os << frame.ToString();
}

struct SettingsFrame final : public FrameInterface {
  absl::Status Deserialize(HPackParser* parser, const FrameHeader& header,
                           absl::BitGenRef bitsrc,
                           SliceBuffer payload) override;
  void Serialize(HPackCompressor* encoder, bool& saw_encoding_errors,
                 BufferPair* out) const override;
  ClientMetadataHandle headers;
  std::string ToString() const override;

  bool operator==(const SettingsFrame&) const { return true; }
};

struct ClientInitialMetadataFrame : public FrameInterface {
  absl::Status Deserialize(HPackParser* parser, const FrameHeader& header,
                           absl::BitGenRef bitsrc,
                           SliceBuffer payload) override;
  void Serialize(HPackCompressor* encoder, bool& saw_encoding_errors,
                 BufferPair* out) const override;
  std::string ToString() const override;

  uint32_t stream_id;
  ClientMetadataHandle headers;
};

struct MessageFrame : public FrameInterface {
  absl::Status Deserialize(HPackParser* parser, const FrameHeader& header,
                           absl::BitGenRef bitsrc,
                           SliceBuffer payload) override;
  void Serialize(HPackCompressor* encoder, bool& saw_encoding_errors,
                 BufferPair* out) const override;
  std::string ToString() const override;

  uint32_t stream_id;
  MessageHandle message;
};

struct ClientEndOfStream : public FrameInterface {
  absl::Status Deserialize(HPackParser* parser, const FrameHeader& header,
                           absl::BitGenRef bitsrc,
                           SliceBuffer payload) override;
  void Serialize(HPackCompressor* encoder, bool& saw_encoding_errors,
                 BufferPair* out) const override;
  std::string ToString() const override;

  uint32_t stream_id;
};

struct ServerInitialMetadataFrame : public FrameInterface {
  absl::Status Deserialize(HPackParser* parser, const FrameHeader& header,
                           absl::BitGenRef bitsrc,
                           SliceBuffer payload) override;
  void Serialize(HPackCompressor* encoder, bool& saw_encoding_errors,
                 BufferPair* out) const override;
  std::string ToString() const override;

  uint32_t stream_id;
  ServerMetadataHandle headers;
};

struct ServerTrailingMetadataFrame : public FrameInterface {
  absl::Status Deserialize(HPackParser* parser, const FrameHeader& header,
                           absl::BitGenRef bitsrc,
                           SliceBuffer payload) override;
  void Serialize(HPackCompressor* encoder, bool& saw_encoding_errors,
                 BufferPair* out) const override;
  std::string ToString() const override;

  uint32_t stream_id;
  ServerMetadataHandle trailers;
};

struct PaddingFrame : public FrameInterface {
  absl::Status Deserialize(HPackParser* parser, const FrameHeader& header,
                           absl::BitGenRef bitsrc,
                           SliceBuffer payload) override;
  void Serialize(HPackCompressor* encoder, bool& saw_encoding_errors,
                 BufferPair* out) const override;
  std::string ToString() const override;

  uint32_t length;
};

struct CancelFrame final : public FrameInterface {
  CancelFrame() = default;
  explicit CancelFrame(uint32_t stream_id) : stream_id(stream_id) {}

  absl::Status Deserialize(HPackParser* parser, const FrameHeader& header,
                           absl::BitGenRef bitsrc,
                           SliceBuffer payload) override;
  void Serialize(HPackCompressor* encoder, bool& saw_encoding_errors,
                 BufferPair* out) const override;
  std::string ToString() const override;

  uint32_t stream_id;

  bool operator==(const CancelFrame& other) const {
    return stream_id == other.stream_id;
  }
};

using ClientFrame = absl::variant<ClientInitialMetadataFrame, MessageFrame,
                                  CancelFrame, PaddingFrame>;
using ServerFrame = absl::variant<ServerInitialMetadataFrame, MessageFrame,
                                  ServerTrailingMetadataFrame, PaddingFrame>;

inline FrameInterface& GetFrameInterface(ClientFrame& frame) {
  return MatchMutable(
      &frame,
      [](ClientInitialMetadataFrame* frame) -> FrameInterface& {
        return *frame;
      },
      [](MessageFrame* frame) -> FrameInterface& { return *frame; },
      [](CancelFrame* frame) -> FrameInterface& { return *frame; });
}

inline FrameInterface& GetFrameInterface(ServerFrame& frame) {
  return MatchMutable(
      &frame,
      [](ServerInitialMetadataFrame* frame) -> FrameInterface& {
        return *frame;
      },
      [](MessageFrame* frame) -> FrameInterface& { return *frame; },
      [](ServerTrailingMetadataFrame* frame) -> FrameInterface& {
        return *frame;
      });
}

}  // namespace chaotic_good
}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_FRAME_H
