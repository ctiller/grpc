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

#ifndef GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_COMMAND_H
#define GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_COMMAND_H

#include "absl/container/flat_hash_map.h"
#include "src/core/ext/transport/chaotic_good/frame.h"
#include "src/core/lib/promise/promise.h"
#include "src/core/util/sync.h"

namespace grpc_core {
namespace chaotic_good {

class CommandStub {
 public:
  template <typename Response>
  struct PendingRequest {
    CommandRequestFrame request_frame;
    Promise<absl::StatusOr<Response>> promise;
  };

  PendingRequest<chaotic_good_frame::MeasureLatencyResponse> MeasureLatency(
      chaotic_good_frame::MeasureLatencyRequest request) {
    return MakePendingRequest("MeasureLatency", std::move(request));
  }

  PendingRequest<chaotic_good_frame::OpenConnectionResponse> OpenConnection(
      chaotic_good_frame::OpenConnectionRequest request) {
    return MakePendingRequest("OpenConnection", std::move(request));
  }

  PendingRequest<chaotic_good_frame::ParkConnectionResponse> ParkConnection(
      chaotic_good_frame::ParkConnectionRequest request) {
    return MakePendingRequest("ParkConnection", std::move(request));
  }

  PendingRequest<chaotic_good_frame::UnparkConnectionResponse> UnparkConnection(
      chaotic_good_frame::UnparkConnectionRequest request) {
    return MakePendingRequest("UnparkConnection", std::move(request));
  }

  PendingRequest<chaotic_good_frame::CloseConnectionResponse> CloseConnection(
      chaotic_good_frame::CloseConnectionRequest request) {
    return MakePendingRequest("CloseConnection", std::move(request));
  }

  absl::Status DispatchResponse(CommandResponseFrame response) {
    MutexLock lock(&mu_);
    auto ex = pending_requests_.extract(response.request_id());
    if (ex.empty()) {
      return absl::InternalError(absl::StrCat(
          "No pending command request with id ", response.request_id()));
    }
    ex.mapped()->Set(std::move(response));
    return absl::OkStatus();
  }

 private:
  template <typename Response, typename Request>
  PendingRequest<Response> MakePendingRequest(absl::string_view command,
                                              Request request) {
    CommandRequestFrame frame;
    frame.set_command(command);
    frame.set_payload(request.SerializeAsString());
    auto latch = AddRequest(frame);
    return PendingRequest<Response> {
      std::move(frame),
          Map(latch->Wait(), [latch](CommandResponseFrame response_frame) {
            Response response;
            bool ok = response.ParseFromArray(response_frame.payload().data(),
                                              response_frame.payload().size());
            if (!ok) {
              return absl::InternalError(
                  "Failed to parse server command response");
            }
            return response;
          });
    }
  }

  std::shared_ptr<InterActivityLatch<CommandResponseFrame>> AddRequest(
      CommandRequestFrame& frame) {
    MutexLock lock(&mu_);
    auto request_id = next_request_id_;
    frame.set_request_id(request_id);
    ++next_request_id_;
    auto latch = std::make_shared<InterActivityLatch<CommandResponseFrame>>();
    pending_requests_.emplace(request_id, latch);
    return latch;
  }

  Mutex mu_;
  uint64_t next_request_id_ ABSL_GUARDED_BY(mu_);
  absl::flat_hash_map<uint64_t,
                      std::shared_ptr<InterActivityLatch<CommandResponseFrame>>>
      pending_requests_ ABSL_GUARDED_BY(mu_);
};

class CommandService {
 public:
};

}  // namespace chaotic_good
}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_COMMAND_H
