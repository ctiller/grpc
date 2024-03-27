// Copyright 2024 The gRPC Authors
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

#ifndef GRPC_SRC_CORE_LIB_EVENT_ENGINE_EXTENSIONS_CHAOTIC_GOOD_EXTENSION_H
#define GRPC_SRC_CORE_LIB_EVENT_ENGINE_EXTENSIONS_CHAOTIC_GOOD_EXTENSION_H

#include <grpc/support/port_platform.h>

#include "absl/strings/string_view.h"

namespace grpc_event_engine {
namespace experimental {

/// An Endpoint extension class that will be supported by EventEngine endpoints
/// which need to work with the ChaoticGood transport.
class ChaoticGoodExtension {
 public:
  virtual ~ChaoticGoodExtension() = default;
  static absl::string_view EndpointExtensionName() {
    return "io.grpc.event_engine.extension.chaotic_good_extension";
  }

  /// If invoked, the endpoint begins collecting TCP stats. If the boolean
  /// arg is_control_channel is true, then the collected stats are grouped into
  /// histograms and counters specific to the chaotic good control channel.
  /// Otherwise they are grouped into histograms and counters specific to the
  /// chaotic good data channel.
  virtual void EnableStatsCollection(bool is_control_channel) = 0;
  /// If invoked, the endpoint tries to preserve proper order and alignment of
  /// any memory that maybe shared across reads.
  virtual void EnforceRxMemoryAlignment() = 0;
};

}  // namespace experimental
}  // namespace grpc_event_engine

#endif  // GRPC_SRC_CORE_LIB_EVENT_ENGINE_EXTENSIONS_CHAOTIC_GOOD_EXTENSION_H
