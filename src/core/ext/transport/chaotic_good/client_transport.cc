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

#include <grpc/support/port_platform.h>

#include "src/core/ext/transport/chaotic_good/client_transport.h"

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "absl/random/bit_gen_ref.h"
#include "absl/random/random.h"
#include "absl/status/statusor.h"

#include <grpc/event_engine/event_engine.h>
#include <grpc/slice.h>
#include <grpc/support/log.h>

#include "src/core/ext/transport/chaotic_good/frame.h"
#include "src/core/ext/transport/chaotic_good/frame_header.h"
#include "src/core/ext/transport/chttp2/transport/hpack_encoder.h"
#include "src/core/lib/gprpp/match.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/iomgr/exec_ctx.h"
#include "src/core/lib/promise/activity.h"
#include "src/core/lib/promise/all_ok.h"
#include "src/core/lib/promise/event_engine_wakeup_scheduler.h"
#include "src/core/lib/promise/loop.h"
#include "src/core/lib/promise/map.h"
#include "src/core/lib/promise/promise.h"
#include "src/core/lib/promise/try_join.h"
#include "src/core/lib/promise/try_seq.h"
#include "src/core/lib/resource_quota/arena.h"
#include "src/core/lib/resource_quota/resource_quota.h"
#include "src/core/lib/slice/slice.h"
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/slice/slice_internal.h"
#include "src/core/lib/transport/promise_endpoint.h"

namespace grpc_core {
namespace chaotic_good {

namespace {
const uint8_t kZeros[64] = {};
}

auto ClientTransport::ReadAndSerializeSomeOutgoingFrames() {
  return Map(
      outgoing_frames_.Next(),
      // Construct data buffers that will be sent to the endpoints.
      [this](ClientFrame client_frame) {
        MatchMutable(
            &client_frame,
            [this](ClientFragmentFrame* frame) mutable {
              if (frame->message.has_value()) {
                // Append message payload to data_endpoint_buffer.
                data_endpoint_write_buffer_.Append(
                    *frame->message->message->payload());
                // Append message padding to data_endpoint_buffer.
                data_endpoint_write_buffer_.Append(
                    Slice::FromStaticBuffer(kZeros, frame->message->padding));
              }
              control_endpoint_write_buffer_.Append(
                  frame->Serialize(&hpack_compressor_));
            },
            [this](CancelFrame* frame) mutable {
              control_endpoint_write_buffer_.Append(
                  frame->Serialize(&hpack_compressor_));
            });
        return Empty{};
      });
}

auto ClientTransport::TransportWriteLoop() {
  return Loop([this] {
    return TrySeq(
        // Get next outgoing frame.
        ReadAndSerializeSomeOutgoingFrames(),
        // Write buffers to corresponding endpoints concurrently.
        [this]() {
          return TryJoin(
              control_endpoint_->Write(
                  std::move(control_endpoint_write_buffer_)),
              data_endpoint_->Write(std::move(data_endpoint_write_buffer_)));
        },
        // Finish writes to difference endpoints and continue the loop.
        []() -> LoopCtl<absl::Status> {
          // The write failures will be caught in TrySeq and exit loop.
          // Therefore, only need to return Continue() in the last lambda
          // function.
          return Continue();
        });
  });
}

auto ClientTransport::ReadFrameBody(Slice read_buffer) {
  frame_header_ =
      FrameHeader::Parse(reinterpret_cast<const uint8_t*>(
                             GRPC_SLICE_START_PTR(read_buffer.c_slice())))
          .value();
  // Read header and trailers from control endpoint.
  // Read message padding and message from data endpoint.
  uint32_t message_padding =
      std::exchange(last_message_padding_, frame_header_.message_padding);
  return TryJoin(control_endpoint_->Read(frame_header_.GetFrameLength()),
                 TrySeq(data_endpoint_->Read(message_padding), [this]() {
                   return data_endpoint_->Read(frame_header_.message_length);
                 }));
}

absl::optional<CallHandler> ClientTransport::LookupStream(uint32_t stream_id) {
  MutexLock lock(&mu_);
  auto it = stream_map_.find(stream_id);
  if (it == stream_map_.end()) {
    return absl::nullopt;
  }
  return it->second;
}

auto ClientTransport::PushFrameIntoCall(ServerFragmentFrame frame,
                                        CallHandler call_handler) {
  auto& headers = frame.headers;
  return TrySeq(
      If(
          headers != nullptr,
          [call_handler, &headers]() mutable {
            return call_handler.PushServerInitialMetadata(std::move(headers));
          },
          []() -> StatusFlag { return Success{}; }),
      [call_handler, message = std::move(frame.message)]() mutable {
        return If(
            message.has_value(),
            [&call_handler, &message]() mutable {
              return call_handler.PushMessage(std::move(message->message));
            },
            []() -> StatusFlag { return Success{}; });
      },
      [call_handler, trailers = std::move(frame.trailers)]() mutable {
        return If(
            trailers != nullptr,
            [&call_handler, &trailers]() mutable {
              return call_handler.PushServerTrailingMetadata(
                  std::move(trailers));
            },
            []() -> StatusFlag { return Success{}; });
      });
}

auto ClientTransport::DeserializeFrameAndPassToCall(SliceBuffer control_buffer,
                                                    SliceBuffer data_buffer,
                                                    CallHandler call_handler) {
  return call_handler.CancelIfFails(
      [this, call_handler, &control_buffer, &data_buffer]() mutable {
        ServerFragmentFrame frame;
        // Deserialize frame from read buffer.
        const auto status =
            frame.Deserialize(&hpack_parser_, frame_header_,
                              absl::BitGenRef(bitgen_), control_buffer);
        return If(
            status.ok(),
            [&frame, &data_buffer, &call_handler, this]() {
              uint32_t message_length = data_buffer.Length();
              frame.message = FragmentMessage(
                  Arena::MakePooled<Message>(std::move(data_buffer), 0), 0,
                  message_length);
              return Map(
                  PushFrameIntoCall(std::move(frame), std::move(call_handler)),
                  [](StatusFlag flag) -> absl::Status {
                    return StatusCast<absl::Status>(flag);
                  });
            },
            [&status] { return Immediate(status); });
      }());
}

auto ClientTransport::MaybeDeserializeFrameAndPassToCall(
    SliceBuffer control_buffer, SliceBuffer data_buffer) {
  absl::optional<CallHandler> call_handler =
      LookupStream(frame_header_.stream_id);
  return If(
      call_handler.has_value(),
      [&call_handler, &control_buffer, &data_buffer, this] {
        return call_handler->SpawnWaitable(
            "deserialize-incoming",
            [this, call_handler = std::move(*call_handler),
             control_buffer = std::move(control_buffer),
             data_buffer = std::move(data_buffer)]() mutable {
              return DeserializeFrameAndPassToCall(std::move(control_buffer),
                                                   std::move(data_buffer),
                                                   std::move(call_handler));
            });
      },
      [] {
        // Stream not found, nothing to do.
        return Immediate(absl::OkStatus());
      });
}

auto ClientTransport::TransportReadLoop() {
  return Loop([this] {
    return TrySeq(
        // Read frame header from control endpoint.
        // TODO(ladynana): remove memcpy in ReadSlice.
        this->control_endpoint_->ReadSlice(FrameHeader::frame_header_size_),
        // Read different parts of the server frame from control/data endpoints
        // based on frame header.
        [this](Slice read_buffer) {
          return ReadFrameBody(std::move(read_buffer));
        },
        // Construct and send the server frame to corresponding stream.
        [this](std::tuple<SliceBuffer, SliceBuffer> ret) mutable {
          return MaybeDeserializeFrameAndPassToCall(
              std::move(std::get<0>(ret)), std::move(std::get<1>(ret)));
        },
        []() -> LoopCtl<absl::Status> { return Continue{}; });
  });
}

auto ClientTransport::OnTransportActivityDone() {
  return [this](absl::Status status) {
    if (!(status.ok() || status.code() == absl::StatusCode::kCancelled)) {
      this->AbortWithError();
    }
  };
}

ClientTransport::ClientTransport(
    std::unique_ptr<PromiseEndpoint> control_endpoint,
    std::unique_ptr<PromiseEndpoint> data_endpoint,
    std::shared_ptr<grpc_event_engine::experimental::EventEngine> event_engine)
    : outgoing_frames_(MpscReceiver<ClientFrame>(4)),
      control_endpoint_(std::move(control_endpoint)),
      data_endpoint_(std::move(data_endpoint)),
      control_endpoint_write_buffer_(SliceBuffer()),
      data_endpoint_write_buffer_(SliceBuffer()),
      event_engine_(event_engine),
      writer_{
          MakeActivity(
              // Continuously write next outgoing frames to promise endpoints.
              TransportWriteLoop(), EventEngineWakeupScheduler(event_engine_),
              OnTransportActivityDone()),
      },
      reader_{MakeActivity(
          // Continuously read next incoming frames from promise endpoints.
          TransportReadLoop(), EventEngineWakeupScheduler(event_engine_),
          OnTransportActivityDone())} {}

ClientTransport::~ClientTransport() {
  if (writer_ != nullptr) {
    writer_.reset();
  }
  if (reader_ != nullptr) {
    reader_.reset();
  }
}

void ClientTransport::AbortWithError() {
  // Mark transport as unavailable when the endpoint write/read failed.
  // Close all the available pipes.
  if (!outgoing_frames_.IsClosed()) {
    outgoing_frames_.MarkClosed();
  }
  ReleasableMutexLock lock(&mu_);
  StreamMap stream_map = std::move(stream_map_);
  stream_map_.clear();
  lock.Release();
  for (const auto& pair : stream_map) {
    auto call_handler = pair.second;
    call_handler.SpawnInfallible("cancel", [call_handler]() mutable {
      call_handler.Cancel(ServerMetadataFromStatus(
          absl::UnavailableError("Transport closed.")));
      return Empty{};
    });
  }
}

uint32_t ClientTransport::MakeStream(CallHandler call_handler) {
  ReleasableMutexLock lock(&mu_);
  const uint32_t stream_id = next_stream_id_++;
  stream_map_.emplace(stream_id, std::move(call_handler));
  lock.Release();
  call_handler.OnDone([this, stream_id]() {
    MutexLock lock(&mu_);
    stream_map_.erase(stream_id);
  });
  return stream_id;
}

auto ClientTransport::CallOutboundLoop(uint32_t stream_id,
                                       CallHandler call_handler) {
  auto send_fragment = [stream_id,
                        outgoing_frames = outgoing_frames_.MakeSender()](
                           ClientFragmentFrame frame) mutable {
    frame.stream_id = stream_id;
    return Map(outgoing_frames.Send(std::move(frame)),
               [](bool success) -> absl::Status {
                 if (!success) {
                   // Failed to send outgoing frame.
                   return absl::UnavailableError("Transport closed.");
                 }
                 return absl::OkStatus();
               });
  };
  return TrySeq(
      // Wait for initial metadata then send it out.
      call_handler.PullClientInitialMetadata(),
      [send_fragment](ClientMetadataHandle md) mutable {
        ClientFragmentFrame frame;
        frame.headers = std::move(md);
        return send_fragment(std::move(frame));
      },
      // Continuously send client frame with client to server messages.
      ForEach(OutgoingMessages(call_handler),
              [send_fragment,
               aligned_bytes = aligned_bytes_](MessageHandle message) mutable {
                ClientFragmentFrame frame;
                // Construct frame header (flags, header_length and
                // trailer_length will be added in serialization).
                const uint32_t message_length = message->payload()->Length();
                const uint32_t padding =
                    message_length % aligned_bytes == 0
                        ? 0
                        : aligned_bytes - message_length % aligned_bytes;
                GPR_ASSERT((message_length + padding) % aligned_bytes == 0);
                frame.message = FragmentMessage(std::move(message), padding,
                                                message_length);
                return send_fragment(std::move(frame));
              }),
      [send_fragment]() mutable {
        ClientFragmentFrame frame;
        frame.end_of_stream = true;
        return send_fragment(std::move(frame));
      });
}

void ClientTransport::StartCall(CallHandler call_handler) {
  // At this point, the connection is set up.
  // Start sending data frames.
  call_handler.SpawnGuarded("outbound_loop", [this, call_handler]() mutable {
    return CallOutboundLoop(MakeStream(call_handler), call_handler);
  });
}

}  // namespace chaotic_good
}  // namespace grpc_core
