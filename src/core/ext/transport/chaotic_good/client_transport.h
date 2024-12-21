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

#include <grpc/event_engine/event_engine.h>
#include <grpc/event_engine/memory_allocator.h>
#include <grpc/grpc.h>
#include <grpc/support/port_platform.h>
#include <stdint.h>
#include <stdio.h>

#include <cstdint>
#include <initializer_list>  // IWYU pragma: keep
#include <map>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "absl/base/thread_annotations.h"
#include "absl/container/flat_hash_map.h"
#include "absl/random/random.h"
#include "absl/status/status.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "src/core/ext/transport/chaotic_good/config.h"
#include "src/core/ext/transport/chaotic_good/frame.h"
#include "src/core/ext/transport/chaotic_good/frame_header.h"
#include "src/core/ext/transport/chaotic_good/frame_transport.h"
#include "src/core/ext/transport/chaotic_good/message_reassembly.h"
#include "src/core/ext/transport/chaotic_good/pending_connection.h"
#include "src/core/lib/promise/activity.h"
#include "src/core/lib/promise/context.h"
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
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/transport/metadata_batch.h"  // IWYU pragma: keep
#include "src/core/lib/transport/promise_endpoint.h"
#include "src/core/lib/transport/transport.h"
#include "src/core/util/sync.h"

namespace grpc_core {
namespace chaotic_good {

class ChaoticGoodClientTransport final : public ClientTransport {
 public:
  ChaoticGoodClientTransport(const ChannelArgs& args,
                             FrameTransport& frame_transport,
                             MessageChunker message_chunker);
  ~ChaoticGoodClientTransport() override;

  FilterStackTransport* filter_stack_transport() override { return nullptr; }
  ClientTransport* client_transport() override { return this; }
  ServerTransport* server_transport() override { return nullptr; }
  absl::string_view GetTransportName() const override { return "chaotic_good"; }
  void SetPollset(grpc_stream*, grpc_pollset*) override {}
  void SetPollsetSet(grpc_stream*, grpc_pollset_set*) override {}
  void PerformOp(grpc_transport_op*) override;
  void Orphan() override;

  void StartCall(CallHandler call_handler) override;
  void AbortWithError();

 private:
  struct Stream : public RefCounted<Stream> {
    explicit Stream(CallHandler call) : call(std::move(call)) {}
    CallHandler call;
    MessageReassembly message_reassembly;
  };
  using StreamMap = absl::flat_hash_map<uint32_t, RefCountedPtr<Stream>>;

  uint32_t MakeStream(CallHandler call_handler);
  RefCountedPtr<Stream> LookupStream(uint32_t stream_id);
  auto CallOutboundLoop(uint32_t stream_id, CallHandler call_handler);
  auto OnTransportActivityDone(absl::string_view what);
  template <typename T>
  auto DispatchFrame(IncomingFrame incoming_frame);
  auto TransportReadLoop(
      FrameTransport::ReadFramePipe::Receiver incoming_frames);
  // Push one frame into a call
  auto PushFrameIntoCall(ServerInitialMetadataFrame frame,
                         RefCountedPtr<Stream> stream);
  auto PushFrameIntoCall(MessageFrame frame, RefCountedPtr<Stream> stream);
  auto PushFrameIntoCall(ServerTrailingMetadataFrame frame,
                         RefCountedPtr<Stream> stream);
  auto PushFrameIntoCall(BeginMessageFrame frame, RefCountedPtr<Stream> stream);
  auto PushFrameIntoCall(MessageChunkFrame frame, RefCountedPtr<Stream> stream);

  std::shared_ptr<grpc_event_engine::experimental::EventEngine> event_engine_;
  grpc_event_engine::experimental::MemoryAllocator allocator_;
  MpscSender<Frame> outgoing_frames_;
  Mutex mu_;
  uint32_t next_stream_id_ ABSL_GUARDED_BY(mu_) = 1;
  // Map of stream incoming server frames, key is stream_id.
  StreamMap stream_map_ ABSL_GUARDED_BY(mu_);
  RefCountedPtr<Party> party_;
  ConnectivityStateTracker state_tracker_ ABSL_GUARDED_BY(mu_){
      "chaotic_good_client", GRPC_CHANNEL_READY};
  MessageChunker message_chunker_;
};

}  // namespace chaotic_good
}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_CLIENT_TRANSPORT_H
