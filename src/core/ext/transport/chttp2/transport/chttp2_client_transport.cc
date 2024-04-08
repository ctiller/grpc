// Copyright 2024 gRPC authors.
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

#include "src/core/ext/transport/chttp2/transport/chttp2_client_transport.h"

#include "src/core/ext/transport/chttp2/transport/frame.h"
#include "src/core/ext/transport/chttp2/transport/hpack_encoder.h"
#include "src/core/lib/promise/for_each.h"
#include "src/core/lib/promise/try_seq.h"

namespace grpc_core {

auto Chttp2ClientTransport::StreamMap::AllocateStreamId(
    CallHandler call_handler) {
  return [this, call_handler = std::move(
                    call_handler)]() -> Poll<absl::StatusOr<StreamId>> {
    if (next_stream_id_ == 0) {
      return absl::UnavailableError("Transport closed or no stream IDs left.");
    }
    if (streams_.size() >= max_concurrent_streams_) {
      return allocate_stream_id_wait_set_.AddPending(
          Activity::current()->MakeNonOwningWaker());
    }
    const uint32_t id = next_stream_id_;
    streams_.emplace(id, std::move(call_handler));
    next_stream_id_ += 2;
    return StreamId{id};
  };
}

void Chttp2ClientTransport::StreamMap::UpdateMaxConcurrentStreams(
    uint32_t max_concurrent_streams) {
  if (max_concurrent_streams > max_concurrent_streams_) {
    allocate_stream_id_wait_set_.WakeupAsync();
  }
  max_concurrent_streams_ = max_concurrent_streams;
}

void Chttp2ClientTransport::WriteQueue::Push(Http2Frame frame) {
  frames_.push_back(std::move(frame));
}

auto Chttp2ClientTransport::CallOutboundLoop(CallHandler call_handler) {
  return TrySeq(
      call_handler.PullClientInitialMetadata(),
      [call_handler, this](ClientMetadataHandle md) {
        return state_.WithLock(
            [call_handler, md = std::move(md)](TransportState& t) mutable {
              return TrySeq(
                  t.stream_map.AllocateStreamId(call_handler),
                  [&t, call_handler, md = std::move(md)](StreamId id) mutable {
                    t.hpack_encoder.EncodeHeaders(
                        HPackCompressor::EncodeHeaderOptions{
                            id.id(), false,
                            t.settings.peer().allow_true_binary_metadata(),
                            t.settings.peer().max_frame_size(), nullptr},
                        *md, t.write_queue.PushBuffer());
                    return Map(ForEach(OutgoingMessages(call_handler),
                                       [&t, id](MessageHandle message) {
                                         t.write_queue.Push(SendMessage{
                                             id, std::move(message)});
                                         return Success{};
                                       }),
                               [id, &t](Success) {
                                 t.write_queue.Push(CloseSends{id});
                                 return Success{};
                               });
                  });
            });
      });
}

void Chttp2ClientTransport::StartCall(CallHandler call_handler) {
  call_handler.SpawnGuarded("outbound_loop",
                            [this, call_handler = std::move(call_handler)] {
                              return CallOutboundLoop(std::move(call_handler));
                            });
}

}  // namespace grpc_core
