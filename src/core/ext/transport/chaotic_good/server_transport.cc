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

#include "src/core/ext/transport/chaotic_good/server_transport.h"

#include <memory>
#include <string>
#include <tuple>

#include "absl/random/bit_gen_ref.h"
#include "absl/random/random.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"

#include <grpc/event_engine/event_engine.h>
#include <grpc/slice.h>
#include <grpc/support/log.h>

#include "src/core/ext/transport/chaotic_good/frame.h"
#include "src/core/ext/transport/chaotic_good/frame_header.h"
#include "src/core/ext/transport/chttp2/transport/hpack_encoder.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/iomgr/exec_ctx.h"
#include "src/core/lib/promise/activity.h"
#include "src/core/lib/promise/event_engine_wakeup_scheduler.h"
#include "src/core/lib/promise/loop.h"
#include "src/core/lib/promise/try_join.h"
#include "src/core/lib/promise/try_seq.h"
#include "src/core/lib/resource_quota/arena.h"
#include "src/core/lib/resource_quota/resource_quota.h"
#include "src/core/lib/slice/slice.h"
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/transport/promise_endpoint.h"

namespace grpc_core {
namespace chaotic_good {

ChaoticGoodServerTransport::ChaoticGoodServerTransport(
    std::unique_ptr<PromiseEndpoint> control_endpoint,
    std::unique_ptr<PromiseEndpoint> data_endpoint,
    std::shared_ptr<grpc_event_engine::experimental::EventEngine> event_engine)
    : control_endpoint_(std::move(control_endpoint)),
      data_endpoint_(std::move(data_endpoint)),
      control_endpoint_write_buffer_(SliceBuffer()),
      data_endpoint_write_buffer_(SliceBuffer()),
      event_engine_(event_engine) {}

ChaoticGoodServerTransport::~ChaoticGoodServerTransport() {
  if (writer_ != nullptr) {
    writer_.reset();
  }
  if (reader_ != nullptr) {
    reader_.reset();
  }
}

auto ChaoticGoodServerTransport::TransportReadLoop() {
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
        [](std::tuple<SliceBuffer, SliceBuffer> ret) mutable {
          return MaybeDeserializeFrameAndPassToCall(
              std::move(std::get<0>(ret)), std::move(std::get<1>(ret)));
        },
        []() -> LoopCtl<absl::Status> { return Continue{}; });
  });
}

void ChaoticGoodServerTransport::AbortWithError() {
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

void ChaoticGoodServerTransport::AddCall(std::shared_ptr<CallInitiator> r) {
  // Server write.
  auto write_loop = Loop([this]() mutable {
    return TrySeq(
        // Get next outgoing frame.
        outgoing_frames_.receiver.Next(),
        // Construct data buffers that will be sent to the endpoints.
        [this](absl::optional<ServerFrame> server_frame) {
          GPR_ASSERT(server_frame.has_value());
          ServerFragmentFrame frame =
              std::move(absl::get<ServerFragmentFrame>(server_frame.value()));
          control_endpoint_write_buffer_.Append(
              frame.Serialize(hpack_compressor_.get()));
          if (frame.message != nullptr) {
            auto frame_header =
                FrameHeader::Parse(
                    reinterpret_cast<const uint8_t*>(GRPC_SLICE_START_PTR(
                        control_endpoint_write_buffer_.c_slice_buffer()
                            ->slices[0])))
                    .value();
            // TODO(ladynana): add message_padding calculation by
            // accumulating bytes sent.
            std::string message_padding(frame_header.message_padding, '0');
            Slice slice(grpc_slice_from_cpp_string(message_padding));
            // Append message payload to data_endpoint_buffer.
            data_endpoint_write_buffer_.Append(std::move(slice));
            // Append message payload to data_endpoint_buffer.
            frame.message->payload()->MoveFirstNBytesIntoSliceBuffer(
                frame.message->payload()->Length(),
                data_endpoint_write_buffer_);
          }
          return absl::OkStatus();
        },
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
  // r->Spawn(std::move(write_loop), [](absl::Status){});
  // Add server write promise.
  auto server_write = Loop([r, this]() mutable {
    return TrySeq(
        // TODO(ladynana): add initial metadata in server frame.
        r->PullServerToClientMessage(),
        [stream_id = r->GetStreamId(), r,
         this](NextResult<MessageHandle> result) mutable {
          bool has_result = result.has_value();
          return If(
              has_result,
              [this, result = std::move(result), stream_id]() mutable {
                std::cout << "write promise get message "
                          << "\n";
                fflush(stdout);
                ServerFragmentFrame frame;
                uint32_t message_length = result.value()->payload()->Length();
                uint32_t message_padding = message_length % aligned_bytes;
                frame.frame_header = FrameHeader{
                    FrameType::kFragment, {}, stream_id, 0, message_length,
                    message_padding,      0};
                frame.message = std::move(result.value());
                return Seq(
                    outgoing_frames_.sender.Push(ServerFrame(std::move(frame))),
                    [](bool success) -> LoopCtl<absl::Status> {
                      if (!success) {
                        // TODO(ladynana): propagate the actual error message
                        // from EventEngine.
                        return absl::UnavailableError(
                            "Transport closed due to endpoint write/read "
                            "failed.");
                      }
                      std::cout << "write promise continue "
                                << "\n";
                      fflush(stdout);
                      return Continue();
                    });
              },
              []() -> LoopCtl<absl::Status> {
                std::cout << "write promise failed "
                          << "\n";
                fflush(stdout);
                return absl::UnavailableError(
                    "Transport closed due to endpoint write/read "
                    "failed.");
              });
        });
  });
  // r->Spawn(std::move(server_write), [](absl::Status){});
  auto stream_id = r->GetStreamId();
  pipe_client_frames_ = std::make_shared<
      InterActivityPipe<ClientFrame, client_frame_queue_size_>>();
  {
    MutexLock lock(&mu_);
    if (stream_map_.count(stream_id) <= 0) {
      stream_map_.insert(
          std::pair<uint32_t,
                    std::shared_ptr<InterActivityPipe<
                        ClientFrame, client_frame_queue_size_>::Sender>>(
              stream_id, std::make_shared<InterActivityPipe<
                             ClientFrame, client_frame_queue_size_>::Sender>(
                             std::move(pipe_client_frames_->sender))));
    }
  }
  auto server_read = Loop([r, this]() mutable {
    return TrySeq(
        pipe_client_frames_->receiver.Next(),
        [r](absl::optional<ClientFrame> client_frame) mutable {
          bool has_frame = client_frame.has_value();
          GPR_ASSERT(r != nullptr);
          return If(
              has_frame,
              [r, client_frame = std::move(client_frame)]() mutable {
                GPR_ASSERT(r != nullptr);
                GPR_ASSERT(client_frame.has_value());
                auto frame = std::move(
                    absl::get<ClientFragmentFrame>(client_frame.value()));
                std::cout << "receive frame from read "
                          << "\n";
                fflush(stdout);
                return Seq(
                    r->PushClientToServerMessage(std::move(frame.message)),
                    [](bool success) -> LoopCtl<absl::Status> {
                      if (!success) {
                        // TODO(ladynana): propagate the actual error message
                        // from EventEngine.
                        return absl::UnavailableError(
                            "Transport closed due to endpoint write/read "
                            "failed.");
                      }
                      std::cout << "read promise continue "
                                << "\n";
                      fflush(stdout);
                      return Continue();
                    });
              },
              []() -> LoopCtl<absl::Status> {
                std::cout << "read clientframe failed "
                          << "\n";
                fflush(stdout);
                return absl::UnavailableError(
                    "Transport closed due to endpoint write/read "
                    "failed.");
              });
        });
  });
  auto call_promise =
      TrySeq(TryJoin(std::move(server_read), std::move(server_write),
                     std::move(write_loop)),
             [](std::tuple<Empty, Empty, Empty>) { return absl::OkStatus(); });
  r->Spawn(std::move(call_promise), [](absl::Status) {});
}
}  // namespace chaotic_good
}  // namespace grpc_core
