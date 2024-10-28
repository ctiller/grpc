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

#ifndef GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_CHAOTIC_GOOD_TRANSPORT_H
#define GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_CHAOTIC_GOOD_TRANSPORT_H

#include <grpc/support/port_platform.h>

#include <cstdint>
#include <utility>

#include "absl/log/log.h"
#include "absl/random/random.h"
#include "absl/strings/escaping.h"
#include "src/core/ext/transport/chaotic_good/frame.h"
#include "src/core/ext/transport/chaotic_good/frame_header.h"
#include "src/core/lib/debug/trace.h"
#include "src/core/lib/event_engine/tcp_socket_utils.h"
#include "src/core/lib/promise/if.h"
#include "src/core/lib/promise/loop.h"
#include "src/core/lib/promise/mpsc.h"
#include "src/core/lib/promise/promise.h"
#include "src/core/lib/promise/try_join.h"
#include "src/core/lib/promise/try_seq.h"
#include "src/core/lib/transport/promise_endpoint.h"

namespace grpc_core {
namespace chaotic_good {

class ChaoticGoodTransport : public RefCounted<ChaoticGoodTransport> {
 public:
  ChaoticGoodTransport(PromiseEndpoint control_endpoint,
                       PromiseEndpoint data_endpoint, uint32_t encode_alignment,
                       uint32_t decode_alignment)
      : control_endpoint_(std::move(control_endpoint)),
        data_endpoint_(std::move(data_endpoint)),
        encode_alignment_(encode_alignment),
        decode_alignment_(decode_alignment) {
    // Enable RxMemoryAlignment and RPC receive coalescing after the transport
    // setup is complete. At this point all the settings frames should have
    // been read.
    data_endpoint_.EnforceRxMemoryAlignmentAndCoalescing();
  }

  auto WriteFrame(const FrameInterface& frame) {
    SliceBuffer control;
    SliceBuffer data;
    FrameHeader header = frame.MakeHeader();
    if (header.payload_length > 128 * 1024) {
      header.payload_connection_id = 1;
      header.Serialize(control.AddTiny(FrameHeader::kFrameHeaderSize));
      frame.SerializePayload(data);
      const size_t padding = header.Padding(encode_alignment_);
      if (padding == 0) {
      } else if (padding < GRPC_SLICE_INLINED_SIZE) {
        memset(data.AddTiny(padding), 0, padding);
      } else {
        auto slice = MutableSlice::CreateUninitialized(padding);
        memset(slice.data(), 0, padding);
        data.AppendIndexed(Slice(std::move(slice)));
      }
    } else {
      header.Serialize(control.AddTiny(FrameHeader::kFrameHeaderSize));
      frame.SerializePayload(control);
    }
    // ignore encoding errors: they will be logged separately already
    GRPC_TRACE_LOG(chaotic_good, INFO)
        << "CHAOTIC_GOOD: WriteFrame to:"
        << ResolvedAddressToString(control_endpoint_.GetPeerAddress())
               .value_or("<<unknown peer address>>")
        << " " << frame.ToString();
    return TryJoin<absl::StatusOr>(control_endpoint_.Write(std::move(control)),
                                   data_endpoint_.Write(std::move(data)));
  }

  template <typename Frame>
  auto TransportWriteLoop(MpscReceiver<Frame>& outgoing_frames) {
    return Loop([self = Ref(), &outgoing_frames] {
      return TrySeq(
          // Get next outgoing frame.
          outgoing_frames.Next(),
          // Serialize and write it out.
          [self = self.get()](Frame client_frame) {
            return self->WriteFrame(GetFrameInterface(client_frame));
          },
          []() -> LoopCtl<absl::Status> {
            // The write failures will be caught in TrySeq and exit loop.
            // Therefore, only need to return Continue() in the last lambda
            // function.
            return Continue();
          });
    });
  }

  // Read frame header and payloads for control and data portions of one frame.
  // Resolves to StatusOr<tuple<FrameHeader, BufferPair>>.
  auto ReadFrameBytes() {
    return TrySeq(
        control_endpoint_.ReadSlice(FrameHeader::kFrameHeaderSize),
        [this](Slice read_buffer) {
          auto frame_header =
              FrameHeader::Parse(reinterpret_cast<const uint8_t*>(
                  GRPC_SLICE_START_PTR(read_buffer.c_slice())));
          GRPC_TRACE_LOG(chaotic_good, INFO)
              << "CHAOTIC_GOOD: ReadHeader from:"
              << ResolvedAddressToString(control_endpoint_.GetPeerAddress())
                     .value_or("<<unknown peer address>>")
              << " "
              << (frame_header.ok() ? frame_header->ToString()
                                    : frame_header.status().ToString());
          return frame_header;
        },
        [this](FrameHeader frame_header) {
          current_frame_header_ = frame_header;
          auto con = frame_header.payload_connection_id == 0
                         ? &control_endpoint_
                         : &data_endpoint_;
          return con->Read(frame_header.payload_length +
                           frame_header.Padding(decode_alignment_));
        },
        [this](SliceBuffer payload)
            -> absl::StatusOr<std::tuple<FrameHeader, SliceBuffer>> {
          payload.RemoveLastNBytes(
              current_frame_header_.Padding(decode_alignment_));
          return std::tuple<FrameHeader, SliceBuffer>(current_frame_header_,
                                                      std::move(payload));
        });
  }

  template <typename T>
  absl::StatusOr<T> DeserializeFrame(const FrameHeader& header,
                                     SliceBuffer payload) {
    T frame;
    GRPC_TRACE_LOG(chaotic_good, INFO)
        << "CHAOTIC_GOOD: Deserialize " << header << " with payload "
        << absl::CEscape(payload.JoinIntoString());
    CHECK_EQ(header.payload_length, payload.Length());
    auto s = frame.Deserialize(header, std::move(payload));
    GRPC_TRACE_LOG(chaotic_good, INFO)
        << "CHAOTIC_GOOD: DeserializeFrame "
        << (s.ok() ? frame.ToString() : s.ToString());
    if (s.ok()) return std::move(frame);
    return std::move(s);
  }

 private:
  PromiseEndpoint control_endpoint_;
  PromiseEndpoint data_endpoint_;
  FrameHeader current_frame_header_;
  const uint32_t encode_alignment_;
  const uint32_t decode_alignment_;
};

}  // namespace chaotic_good
}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_CHAOTIC_GOOD_TRANSPORT_H
