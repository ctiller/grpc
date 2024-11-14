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

#ifndef GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_LISTENER_H
#define GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_LISTENER_H

#include <string>

#include "absl/status/statusor.h"
#include "src/core/lib/promise/latch.h"
#include "src/core/lib/transport/promise_endpoint.h"

namespace grpc_core {
namespace chaotic_good {

class PendingServerConnection {
 public:
  absl::string_view id() { return id_; }
  auto Await() { return latch_.Wait(); }

 private:
  const std::string id_;
  Latch<absl::StatusOr<PromiseEndpoint>> latch_;
};

class ChaoticGoodListener {
 public:
  virtual PendingServerConnection RequestDataConnection() = 0;
};

}  // namespace chaotic_good
}  // namespace grpc_core

#endif
