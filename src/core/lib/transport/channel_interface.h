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

#ifndef GRPC_SRC_CORE_LIB_TRANSPORT_CHANNEL_INTERFACE_H
#define GRPC_SRC_CORE_LIB_TRANSPORT_CHANNEL_INTERFACE_H

namespace grpc_core {

class ChannelInterface {
 public:
  // Returns an arena that can be used to allocate memory for initial metadata
  // parsing, and later passed to CreateCall() as the underlying arena for
  // that call.
  Arena* CreateArena() = 0;
  // Destroy a previously created arena
  void DestroyArena(Arena* arena) = 0;
  // Create a call at the server (or fail)
  // arena must have been previously allocated by CreateArena()
  virtual absl::StatusOr<CallInitiator> CreateCall(
      ClientMetadataHandle client_initial_metadata, Arena* arena) = 0;
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_TRANSPORT_CHANNEL_INTERFACE_H
