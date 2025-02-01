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

#ifndef GRPC2_SUPPORT_CHANNEL_INTERFACE_H
#define GRPC2_SUPPORT_CHANNEL_INTERFACE_H

#include "src/core/lib/transport/message.h"
#include "src/core/lib/transport/metadata.h"
#include "src/core/util/dual_ref_counted.h"

namespace grpc2 {

class ClientCallInterface
    : public grpc_core::DualRefCounted<ClientCallInterface,
                                       grpc_core::PolymorphicRefCount,
                                       grpc_core::UnrefCallDestroy> {
 public:
  virtual void PerformUnary(
      absl::AnyInvocable<
          void(absl::optional<grpc_core::ServerMetadataHandle>,
               absl::optional<grpc_core::MessageHandle>,
               absl::optional<grpc_core::ServerMetadataHandle>)>) = 0;
};

class ChannelInterface {
 public:
  virtual grpc_core::RefCountedPtr<ClientCallInterface> NewCall(
      grpc_core::ClientMetadataHandle metadata) = 0;
};

}  // namespace grpc2

#endif  // GRPC2_SUPPORT_CHANNEL_INTERFACE_H