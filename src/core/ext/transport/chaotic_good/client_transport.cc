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
#include "src/core/lib/promise/event_engine_wakeup_scheduler.h"
#include "src/core/lib/promise/loop.h"
#include "src/core/lib/promise/map.h"
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

auto ClientTransport::TransportWriteLoop() {
  return Loop([this] {
    return TrySeq(
        // Get next outgoing frame.
        Map(outgoing_frames_.Next(),
            // Construct data buffers that will be sent to the endpoints.
            [this](ClientFrame client_frame) {
              MatchMutable(
                  &client_frame,
                  [this](ClientFragmentFrame* frame) mutable {
                    control_endpoint_write_buffer_.Append(
                        frame->Serialize(&hpack_compressor_));
                    if (frame->message != nullptr) {
                      // Append message padding to data_endpoint_buffer.
                      data_endpoint_write_buffer_.Append(
                          Slice::FromStaticBuffer(kZeros,
                                                  frame->message_padding));
                      // Append message payload to data_endpoint_buffer.
                      frame->message->payload()->MoveFirstNBytesIntoSliceBuffer(
                          frame->message->payload()->Length(),
                          data_endpoint_write_buffer_);
                    }
                  },
                  [this](CancelFrame* frame) mutable {
                    control_endpoint_write_buffer_.Append(
                        frame->Serialize(&hpack_compressor_));
                  });
              return Empty{};
            }),
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

auto ClientTransport::TransportReadLoop() {
  return Loop([this] {
    return TrySeq(
        // Read frame header from control endpoint.
        // TODO(ladynana): remove memcpy in ReadSlice.
        this->control_endpoint_->ReadSlice(FrameHeader::frame_header_size_),
        // Read different parts of the server frame from control/data endpoints
        // based on frame header.
        [this](Slice read_buffer) mutable {
          frame_header_ = FrameHeader::Parse(
                              reinterpret_cast<const uint8_t*>(
                                  GRPC_SLICE_START_PTR(read_buffer.c_slice())))
                              .value();
          // Read header and trailers from control endpoint.
          // Read message padding and message from data endpoint.
          return TryJoin(
              control_endpoint_->Read(frame_header_.GetFrameLength()),
              TrySeq(data_endpoint_->Read(frame_header_.message_padding),
                     data_endpoint_->Read(frame_header_.message_length)));
        },
        // Construct and send the server frame to corresponding stream.
        [this](std::tuple<SliceBuffer, SliceBuffer> ret) mutable {
          auto& control_endpoint_read_buffer = std::get<0>(ret);
          auto& data_endpoint_read_buffer = std::get<1>(ret);
          ServerFragmentFrame frame;
          // Deserialize frame from read buffer.
          const auto status = frame.Deserialize(&hpack_parser_, frame_header_,
                                                absl::BitGenRef(bitgen_),
                                                control_endpoint_read_buffer);
          using PushType = decltype(std::declval<FrameSender>().Push(
              std::declval<ServerFrame>()));
          auto sender = [this, &frame, &status]() -> absl::StatusOr<PushType> {
            if (!status.ok()) return status;
            MutexLock lock(&mu_);
            auto it = stream_map_.find(frame.stream_id);
            if (it == stream_map_.end()) {
              return absl::InternalError("Stream not found.");
            }
            return it->second.Push(std::move(frame));
          }();
          // Move message into frame.
          if (sender.ok()) {
            frame.message = Arena::MakePooled<Message>(
                std::move(data_endpoint_read_buffer), 0);
          }
          return If(
              sender.ok(),
              [&sender]() mutable {
                return Map(std::move(*sender), [](bool ret) {
                  return ret ? absl::OkStatus() : absl::CancelledError();
                });
              },
              [&sender]() { return sender.status(); });
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
  MutexLock lock(&mu_);
  for (auto& pair : stream_map_) {
    if (!pair.second.IsClosed()) {
      pair.second.MarkClosed();
    }
  }
}

ClientTransport::NewStream ClientTransport::MakeStream() {
  FramePipe pipe_server_frames;
  MutexLock lock(&mu_);
  const uint32_t stream_id = next_stream_id_++;
  stream_map_.emplace(stream_id, std::move(pipe_server_frames.sender));
  return NewStream{stream_id, std::move(pipe_server_frames.receiver)};
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
                uint32_t message_length = message->payload()->Length();
                frame.message_padding = message_length % aligned_bytes;
                frame.message = std::move(message);
                return send_fragment(std::move(frame));
              }),
      [send_fragment]() mutable {
        ClientFragmentFrame frame;
        frame.end_of_stream = true;
        return send_fragment(std::move(frame));
      });
}

auto ClientTransport::CallInboundLoop(CallHandler call_handler,
                                      FrameReceiver receiver) {
  return Loop([receiver = std::move(receiver),
               call_handler = std::move(call_handler)]() mutable {
    return TrySeq(
        Map(receiver.Next(),
            [](absl::optional<ServerFrame> server_frame)
                -> absl::StatusOr<ServerFrame> {
              if (!server_frame.has_value()) {
                return absl::UnavailableError("Transport closed.");
              }
              return std::move(*server_frame);
            }),
        [call_handler](ServerFrame server_frame) {
          auto frame = absl::get<ServerFragmentFrame>(std::move(server_frame));
          bool has_headers = (frame.headers != nullptr);
          bool has_message = (frame.message != nullptr);
          bool has_trailers = (frame.trailers != nullptr);
          return TrySeq(
              TryJoin(If(
                          has_headers,
                          [call_handler,
                           headers = std::move(frame.headers)]() mutable {
                            return call_handler.PushServerInitialMetadata(
                                std::move(headers));
                          },
                          []() -> StatusFlag { return Success{}; }),
                      If(
                          has_message,
                          [call_handler,
                           message = std::move(frame.message)]() mutable {
                            return call_handler.PushMessage(std::move(message));
                          },
                          []() -> StatusFlag { return Success{}; })),
              If(
                  has_trailers,
                  [call_handler,
                   trailers = std::move(frame.trailers)]() mutable {
                    return Map(call_handler.PushServerTrailingMetadata(
                                   std::move(trailers)),
                               [](StatusFlag f) -> LoopCtl<absl::Status> {
                                 return StatusCast<absl::Status>(f);
                               });
                  },
                  []() -> LoopCtl<absl::Status> { return Continue{}; }));
        });
  });
}

void ClientTransport::StartCall(CallHandler call_handler) {
  // At this point, the connection is set up.
  // Start sending data frames.
  NewStream stream = MakeStream();
  call_handler.SpawnGuarded(
      "outbound_loop",
      [this, stream_id = stream.stream_id, call_handler]() mutable {
        return CallOutboundLoop(stream_id, std::move(call_handler));
      });
  call_handler.SpawnGuarded(
      "inbound_loop", [this, call_handler = std::move(call_handler),
                       receiver = std::move(stream.receiver)]() mutable {
        return CallInboundLoop(std::move(call_handler), std::move(receiver));
      });
}

}  // namespace chaotic_good
}  // namespace grpc_core
