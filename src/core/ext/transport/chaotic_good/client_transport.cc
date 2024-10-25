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

#include "src/core/ext/transport/chaotic_good/client_transport.h"

#include <grpc/event_engine/event_engine.h>
#include <grpc/slice.h>
#include <grpc/support/port_platform.h>

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/random/bit_gen_ref.h"
#include "absl/random/random.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "src/core/ext/transport/chaotic_good/chaotic_good_transport.h"
#include "src/core/ext/transport/chaotic_good/frame.h"
#include "src/core/ext/transport/chaotic_good/frame_header.h"
#include "src/core/ext/transport/chttp2/transport/hpack_encoder.h"
#include "src/core/lib/event_engine/event_engine_context.h"
#include "src/core/lib/event_engine/extensions/tcp_trace.h"
#include "src/core/lib/event_engine/query_extensions.h"
#include "src/core/lib/iomgr/exec_ctx.h"
#include "src/core/lib/promise/loop.h"
#include "src/core/lib/promise/map.h"
#include "src/core/lib/promise/switch.h"
#include "src/core/lib/promise/try_seq.h"
#include "src/core/lib/resource_quota/arena.h"
#include "src/core/lib/resource_quota/resource_quota.h"
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/transport/promise_endpoint.h"
#include "src/core/util/ref_counted_ptr.h"

namespace grpc_core {
namespace chaotic_good {

void ChaoticGoodClientTransport::Orphan() {
  AbortWithError();
  RefCountedPtr<Party> party;
  {
    MutexLock lock(&mu_);
    party = std::move(party_);
  }
  party.reset();
  Unref();
}

auto ChaoticGoodClientTransport::TransportWriteLoop(
    RefCountedPtr<ChaoticGoodTransport> transport) {
  return Loop([this, transport = std::move(transport)] {
    return TrySeq(
        // Get next outgoing frame.
        outgoing_frames_.Next(),
        // Serialize and write it out.
        [transport = transport.get()](ClientFrame client_frame) {
          return transport->WriteFrame(GetFrameInterface(client_frame));
        },
        []() -> LoopCtl<absl::Status> {
          // The write failures will be caught in TrySeq and exit loop.
          // Therefore, only need to return Continue() in the last lambda
          // function.
          return Continue();
        });
  });
}

absl::optional<CallHandler> ChaoticGoodClientTransport::LookupStream(
    uint32_t stream_id) {
  MutexLock lock(&mu_);
  auto it = stream_map_.find(stream_id);
  if (it == stream_map_.end()) {
    return absl::nullopt;
  }
  return it->second;
}

auto ChaoticGoodClientTransport::PushFrameIntoCall(
    ServerInitialMetadataFrame frame, CallHandler call_handler) {
  return Immediate(
      call_handler.PushServerInitialMetadata(std::move(frame.headers)));
}

auto ChaoticGoodClientTransport::PushFrameIntoCall(MessageFrame frame,
                                                   CallHandler call_handler) {
  return call_handler.PushMessage(std::move(frame.message));
}

auto ChaoticGoodClientTransport::PushFrameIntoCall(
    ServerTrailingMetadataFrame frame, CallHandler call_handler) {
  call_handler.PushServerTrailingMetadata(std::move(frame.trailers));
  return Immediate(Success{});
}

template <typename T>
auto ChaoticGoodClientTransport::DispatchFrame(ChaoticGoodTransport* transport,
                                               const FrameHeader& header,
                                               SliceBuffer payload) {
  return TrySeq(
      [transport, header, payload = std::move(payload)]() mutable {
        return transport->DeserializeFrame<T>(header, std::move(payload));
      },
      [this](T frame) {
        absl::optional<CallHandler> call_handler =
            LookupStream(frame.stream_id);
        return If(
            call_handler.has_value(),
            [this, &call_handler, &frame]() {
              return call_handler->SpawnWaitable(
                  "push-frame", [this, call_handler = *call_handler,
                                 frame = std::move(frame)]() mutable {
                    return Map(call_handler.CancelIfFails(PushFrameIntoCall(
                                   std::move(frame), call_handler)),
                               [](StatusFlag) { return absl::OkStatus(); });
                  });
            },
            []() { return absl::OkStatus(); });
      });
}

auto ChaoticGoodClientTransport::TransportReadLoop(
    RefCountedPtr<ChaoticGoodTransport> transport) {
  return Loop([this, transport = std::move(transport)] {
    return TrySeq(
        transport->ReadFrameBytes(),
        [this, transport = transport.get()](
            std::tuple<FrameHeader, SliceBuffer> frame_bytes) {
          const auto& header = std::get<0>(frame_bytes);
          SliceBuffer& payload = std::get<1>(frame_bytes);
          return Switch(
              header.type,
              Case<FrameType, FrameType::kServerInitialMetadata>([&, this]() {
                return DispatchFrame<ServerInitialMetadataFrame>(
                    transport, header, std::move(payload));
              }),
              Case<FrameType, FrameType::kServerTrailingMetadata>([&, this]() {
                return DispatchFrame<ServerTrailingMetadataFrame>(
                    transport, header, std::move(payload));
              }),
              Case<FrameType, FrameType::kMessage>([&, this]() {
                return DispatchFrame<MessageFrame>(transport, header,
                                                   std::move(payload));
              }),
              Default([&]() {
                LOG_EVERY_N_SEC(INFO, 10)
                    << "Bad frame type: " << header.ToString();
                return absl::OkStatus();
              }));
        },
        []() -> LoopCtl<absl::Status> { return Continue{}; });
  });
}

auto ChaoticGoodClientTransport::OnTransportActivityDone(
    absl::string_view what) {
  return [self = RefAsSubclass<ChaoticGoodClientTransport>(),
          what](absl::Status status) {
    GRPC_TRACE_LOG(chaotic_good, INFO)
        << "CHAOTIC_GOOD: Client transport " << self.get() << " closed (via "
        << what << "): " << status;
    self->AbortWithError();
  };
}

ChaoticGoodClientTransport::ChaoticGoodClientTransport(
    PromiseEndpoint control_endpoint, PromiseEndpoint data_endpoint,
    const ChannelArgs& args,
    std::shared_ptr<grpc_event_engine::experimental::EventEngine> event_engine,
    HPackParser hpack_parser, HPackCompressor hpack_encoder)
    : allocator_(args.GetObject<ResourceQuota>()
                     ->memory_quota()
                     ->CreateMemoryAllocator("chaotic-good")),
      outgoing_frames_(4) {
  // Set up TCP tracer if enabled.
  if (args.GetBool(GRPC_ARG_TCP_TRACING_ENABLED).value_or(false)) {
    auto* epte = grpc_event_engine::experimental::QueryExtension<
        grpc_event_engine::experimental::TcpTraceExtension>(
        data_endpoint.GetEventEngineEndpoint().get());
    if (epte != nullptr) {
      epte->InitializeAndReturnTcpTracer();
    }
  }
  auto transport = MakeRefCounted<ChaoticGoodTransport>(
      std::move(control_endpoint), std::move(data_endpoint),
      std::move(hpack_parser), std::move(hpack_encoder), 64, 64);
  auto party_arena = SimpleArenaAllocator(0)->MakeArena();
  party_arena->SetContext<grpc_event_engine::experimental::EventEngine>(
      event_engine.get());
  party_ = Party::Make(std::move(party_arena));
  party_->Spawn("client-chaotic-writer",
                GRPC_LATENT_SEE_PROMISE("ClientTransportWriteLoop",
                                        TransportWriteLoop(transport)),
                OnTransportActivityDone("write_loop"));
  party_->Spawn(
      "client-chaotic-reader",
      GRPC_LATENT_SEE_PROMISE("ClientTransportReadLoop",
                              TransportReadLoop(std::move(transport))),
      OnTransportActivityDone("read_loop"));
}

ChaoticGoodClientTransport::~ChaoticGoodClientTransport() { party_.reset(); }

void ChaoticGoodClientTransport::AbortWithError() {
  // Mark transport as unavailable when the endpoint write/read failed.
  // Close all the available pipes.
  outgoing_frames_.MarkClosed();
  ReleasableMutexLock lock(&mu_);
  StreamMap stream_map = std::move(stream_map_);
  stream_map_.clear();
  lock.Release();
  for (const auto& pair : stream_map) {
    auto call_handler = pair.second;
    call_handler.SpawnInfallible("cancel", [call_handler]() mutable {
      call_handler.PushServerTrailingMetadata(ServerMetadataFromStatus(
          absl::UnavailableError("Transport closed.")));
      return Empty{};
    });
  }
}

uint32_t ChaoticGoodClientTransport::MakeStream(CallHandler call_handler) {
  MutexLock lock(&mu_);
  const uint32_t stream_id = next_stream_id_++;
  const bool on_done_added =
      call_handler.OnDone([self = RefAsSubclass<ChaoticGoodClientTransport>(),
                           stream_id](bool cancelled) {
        if (cancelled) {
          self->outgoing_frames_.MakeSender().UnbufferedImmediateSend(
              CancelFrame{stream_id});
        }
        MutexLock lock(&self->mu_);
        self->stream_map_.erase(stream_id);
      });
  if (!on_done_added) return 0;
  stream_map_.emplace(stream_id, call_handler);
  return stream_id;
}

namespace {
absl::Status BooleanSuccessToTransportError(bool success) {
  return success ? absl::OkStatus()
                 : absl::UnavailableError("Transport closed.");
}
}  // namespace

auto ChaoticGoodClientTransport::CallOutboundLoop(uint32_t stream_id,
                                                  CallHandler call_handler) {
  auto send_fragment = [stream_id,
                        outgoing_frames =
                            outgoing_frames_.MakeSender()](auto frame) mutable {
    frame.stream_id = stream_id;
    return Map(outgoing_frames.Send(std::move(frame)),
               BooleanSuccessToTransportError);
  };
  auto send_fragment_acked = [stream_id,
                              outgoing_frames = outgoing_frames_.MakeSender()](
                                 auto frame) mutable {
    frame.stream_id = stream_id;
    return Map(outgoing_frames.SendAcked(std::move(frame)),
               BooleanSuccessToTransportError);
  };
  return GRPC_LATENT_SEE_PROMISE(
      "CallOutboundLoop",
      TrySeq(
          // Wait for initial metadata then send it out.
          call_handler.PullClientInitialMetadata(),
          [send_fragment](ClientMetadataHandle md) mutable {
            GRPC_TRACE_LOG(chaotic_good, INFO)
                << "CHAOTIC_GOOD: Sending initial metadata: "
                << md->DebugString();
            ClientInitialMetadataFrame frame;
            frame.headers = std::move(md);
            return send_fragment(std::move(frame));
          },
          // Continuously send client frame with client to server messages.
          ForEach(OutgoingMessages(call_handler),
                  [send_fragment_acked](MessageHandle message) mutable {
                    MessageFrame frame;
                    frame.message = std::move(message);
                    return send_fragment_acked(std::move(frame));
                  }),
          [send_fragment]() mutable {
            ClientEndOfStream frame;
            return send_fragment(std::move(frame));
          }));
}

void ChaoticGoodClientTransport::StartCall(CallHandler call_handler) {
  // At this point, the connection is set up.
  // Start sending data frames.
  call_handler.SpawnGuarded(
      "outbound_loop", [self = RefAsSubclass<ChaoticGoodClientTransport>(),
                        call_handler]() mutable {
        const uint32_t stream_id = self->MakeStream(call_handler);
        return If(
            stream_id != 0,
            [stream_id, call_handler = std::move(call_handler),
             self = std::move(self)]() {
              return Map(
                  self->CallOutboundLoop(stream_id, call_handler),
                  [stream_id, sender = self->outgoing_frames_.MakeSender()](
                      absl::Status result) mutable {
                    GRPC_TRACE_LOG(chaotic_good, INFO)
                        << "CHAOTIC_GOOD: Call " << stream_id
                        << " finished with " << result.ToString();
                    if (!result.ok()) {
                      GRPC_TRACE_LOG(chaotic_good, INFO)
                          << "CHAOTIC_GOOD: Send cancel";
                      if (!sender.UnbufferedImmediateSend(
                              CancelFrame{stream_id})) {
                        GRPC_TRACE_LOG(chaotic_good, INFO)
                            << "CHAOTIC_GOOD: Send cancel failed";
                      }
                    }
                    return result;
                  });
            },
            []() { return absl::OkStatus(); });
      });
}

void ChaoticGoodClientTransport::PerformOp(grpc_transport_op* op) {
  MutexLock lock(&mu_);
  bool did_stuff = false;
  if (op->start_connectivity_watch != nullptr) {
    state_tracker_.AddWatcher(op->start_connectivity_watch_state,
                              std::move(op->start_connectivity_watch));
    did_stuff = true;
  }
  if (op->stop_connectivity_watch != nullptr) {
    state_tracker_.RemoveWatcher(op->stop_connectivity_watch);
    did_stuff = true;
  }
  if (op->set_accept_stream) {
    Crash("set_accept_stream not supported on clients");
  }
  if (!did_stuff) {
    Crash(absl::StrCat("unimplemented transport perform op: ",
                       grpc_transport_op_string(op)));
  }
  ExecCtx::Run(DEBUG_LOCATION, op->on_consumed, absl::OkStatus());
}

}  // namespace chaotic_good
}  // namespace grpc_core
