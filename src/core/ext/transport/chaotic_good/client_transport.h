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

#ifndef GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_CLIENT_TRANSPORT_H
#define GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_CLIENT_TRANSPORT_H

#include <grpc/support/port_platform.h>

#include <stdint.h>
#include <stdio.h>

#include <initializer_list>  // IWYU pragma: keep
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "absl/base/thread_annotations.h"
#include "absl/status/status.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"

#include <grpc/event_engine/event_engine.h>
#include <grpc/event_engine/memory_allocator.h>
#include <grpc/status.h>

#include "src/core/ext/transport/chaotic_good/frame.h"
#include "src/core/ext/transport/chaotic_good/frame_header.h"
#include "src/core/ext/transport/chttp2/transport/hpack_encoder.h"
#include "src/core/ext/transport/chttp2/transport/hpack_parser.h"
#include "src/core/lib/gprpp/sync.h"
#include "src/core/lib/gprpp/time.h"
#include "src/core/lib/promise/activity.h"
#include "src/core/lib/promise/for_each.h"
#include "src/core/lib/promise/if.h"
#include "src/core/lib/promise/inter_activity_pipe.h"
#include "src/core/lib/promise/loop.h"
#include "src/core/lib/promise/mpsc.h"
#include "src/core/lib/promise/pipe.h"
#include "src/core/lib/promise/poll.h"
#include "src/core/lib/promise/try_join.h"
#include "src/core/lib/promise/try_seq.h"
#include "src/core/lib/resource_quota/arena.h"
#include "src/core/lib/resource_quota/memory_quota.h"
#include "src/core/lib/slice/slice.h"
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/transport/error_utils.h"
#include "src/core/lib/transport/metadata_batch.h"
#include "src/core/lib/transport/promise_endpoint.h"
#include "src/core/lib/transport/transport.h"

namespace grpc_core {
namespace chaotic_good {

class ClientTransport {
 public:
  ClientTransport(std::unique_ptr<PromiseEndpoint> control_endpoint,
                  std::unique_ptr<PromiseEndpoint> data_endpoint,
                  std::shared_ptr<grpc_event_engine::experimental::EventEngine>
                      event_engine);
  ~ClientTransport() {
    if (writer_ != nullptr) {
      writer_.reset();
    }
    if (reader_ != nullptr) {
      reader_.reset();
    }
  }
  void AbortWithError() {
    // Transport becomes unavailable if the endpoint write/read failed,
    // therefore, stop writer_/reader_ activity and abort on-going stream calls.
    std::cout << "\n close outgoing frame.";
    fflush(stdout);
    if (outgoing_frames_ != nullptr) {
      outgoing_frames_->MarkClosed();
    }
    // Handle the promise endpoint read failures with
    // iterating stream_map_ and close all the pipes once available.
    std::map<uint32_t, std::shared_ptr<InterActivityPipe<
                           ServerFrame, server_frame_queue_size_>::Sender>>
        stream_map;
    {
      MutexLock lock(&mu_);
      stream_map = stream_map_;
    }
    for (const auto& i : stream_map) {
      std::cout << "\n close sender.";
      fflush(stdout);
      i.second->MarkClose();
    }
  }
  auto AddStream(CallArgs call_args) {
    // At this point, the connection is set up.
    // Start sending data frames.
    uint64_t stream_id;
    InterActivityPipe<ServerFrame, server_frame_queue_size_> server_frames;
    {
      MutexLock lock(&mu_);
      stream_id = next_stream_id_++;
      stream_map_.insert(
          std::pair<uint32_t,
                    std::shared_ptr<InterActivityPipe<
                        ServerFrame, server_frame_queue_size_>::Sender>>(
              stream_id, std::make_shared<InterActivityPipe<
                             ServerFrame, server_frame_queue_size_>::Sender>(
                             std::move(server_frames.sender))));
    }
    auto outgoing_frame = outgoing_frames_->MakeSender();
    return TrySeq(
        TryJoin(
            // Continuously send client frame with client to server messages.
            ForEach(std::move(*call_args.client_to_server_messages),
                    [stream_id, initial_frame = true,
                     client_initial_metadata =
                         std::move(call_args.client_initial_metadata),
                     outgoing_frame = std::move(outgoing_frame)](
                        MessageHandle result) mutable {
                      ClientFragmentFrame frame;
                      frame.stream_id = stream_id;
                      frame.message = std::move(result);
                      if (initial_frame) {
                        // Send initial frame with client intial metadata.
                        frame.headers = std::move(client_initial_metadata);
                        initial_frame = false;
                      }
                      return TrySeq(
                          outgoing_frame.Send(ClientFrame(std::move(frame))),
                          [](bool success) -> absl::Status {
                            if (!success) {
                              return absl::InternalError(
                                  "Send frame to outgoing_frames failed.");
                            }
                            std::cout << "\n write send frame done.";
                            fflush(stdout);
                            return absl::OkStatus();
                          });
                    }),
            // Continuously receive server frames from endpoints and save
            // results to call_args.
            Loop([server_initial_metadata = call_args.server_initial_metadata,
                  server_to_client_messages =
                      call_args.server_to_client_messages,
                  receiver = std::move(server_frames.receiver),
                  this]() mutable {
              return TrySeq(
                  // Receive incoming server frame.
                  receiver.Next(),
                  // Save incomming frame results to call_args.
                  [server_initial_metadata, server_to_client_messages,
                   this](absl::optional<ServerFrame> server_frame) mutable {
                    bool transport_closed = false;
                    std::shared_ptr<ServerFragmentFrame> frame =
                        std::make_shared<ServerFragmentFrame>();
                    if (!server_frame.has_value()) {
                      // Server frames pipe is closed by sender, return
                      // ServerMetadata with error.
                      transport_closed = true;
                    } else {
                      frame = std::make_shared<ServerFragmentFrame>(std::move(
                          absl::get<ServerFragmentFrame>(*server_frame)));
                    }
                    return TrySeq(
                        If((frame != nullptr && frame->headers != nullptr),
                           [server_initial_metadata,
                            headers = std::move(frame->headers)]() mutable {
                             return server_initial_metadata->Push(
                                 std::move(headers));
                           },
                           [] { return false; }),
                        If((frame != nullptr && frame->message != nullptr),
                           [server_to_client_messages,
                            message = std::move(frame->message)]() mutable {
                             return server_to_client_messages->Push(
                                 std::move(message));
                           },
                           [] { return false; }),
                        If((frame != nullptr && frame->trailers != nullptr),
                           [trailers = std::move(frame->trailers)]() mutable
                           -> LoopCtl<ServerMetadataHandle> {
                             return std::move(trailers);
                           },
                           [transport_closed,
                            this]() mutable -> LoopCtl<ServerMetadataHandle> {
                             if (transport_closed) {
                               auto trailers =
                                   arena_->MakePooled<ServerMetadata>(
                                       arena_.get());
                               grpc_status_code code;
                               std::string message;
                               grpc_error_get_status(
                                   absl::UnavailableError(
                                       "Transport is unavailable."),
                                   Timestamp::InfFuture(), &code, &message,
                                   nullptr, nullptr);
                               trailers->Set(GrpcStatusMetadata(), code);
                               trailers->Set(GrpcMessageMetadata(),
                                             Slice::FromCopiedString(message));
                               std::cout << "\n transport close.";
                               fflush(stdout);
                               return std::move(trailers);
                             }
                             return Continue();
                           }));
                  });
            })),
        [](std::tuple<Empty, ServerMetadataHandle> ret) {
          std::cout << "\n write/read frame done.";
          fflush(stdout);
          return std::move(std::get<1>(ret));
        });
  }

 private:
  // Max buffer is set to 4, so that for stream writes each time it will queue
  // at most 2 frames.
  std::shared_ptr<MpscReceiver<ClientFrame>> outgoing_frames_;
  // Queue size of each stream pipe is set to 2, so that for each stream read it
  // will queue at most 2 frames.
  static const size_t server_frame_queue_size_ = 2;
  Mutex mu_;
  uint32_t next_stream_id_ ABSL_GUARDED_BY(mu_) = 1;
  // Map of stream incoming server frames, key is stream_id.
  std::map<uint32_t, std::shared_ptr<InterActivityPipe<
                         ServerFrame, server_frame_queue_size_>::Sender>>
      stream_map_ ABSL_GUARDED_BY(mu_);
  ActivityPtr writer_;
  ActivityPtr reader_;
  std::unique_ptr<PromiseEndpoint> control_endpoint_;
  std::unique_ptr<PromiseEndpoint> data_endpoint_;
  SliceBuffer control_endpoint_write_buffer_;
  SliceBuffer data_endpoint_write_buffer_;
  SliceBuffer control_endpoint_read_buffer_;
  SliceBuffer data_endpoint_read_buffer_;
  std::unique_ptr<HPackCompressor> hpack_compressor_;
  std::unique_ptr<HPackParser> hpack_parser_;
  std::shared_ptr<FrameHeader> frame_header_;
  MemoryAllocator memory_allocator_;
  std::shared_ptr<Arena> arena_;  // Shared ownership with segment frames.
  // Use to synchronize writer_ and reader_ activity with outside activities;
  std::shared_ptr<grpc_event_engine::experimental::EventEngine> event_engine_;
};

}  // namespace chaotic_good
}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_CLIENT_TRANSPORT_H