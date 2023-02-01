// Copyright 2023 gRPC authors.
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

#ifndef GRPC_TEST_CORE_FILTERS_FILTER_TEST_H
#define GRPC_TEST_CORE_FILTERS_FILTER_TEST_H

#include <stddef.h>
#include <stdint.h>

#include <initializer_list>
#include <iosfwd>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "absl/strings/escaping.h"
#include "absl/strings/string_view.h"
#include "gmock/gmock.h"

#include <grpc/event_engine/memory_allocator.h>

#include "src/core/lib/channel/promise_based_filter.h"
#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/resource_quota/memory_quota.h"
#include "src/core/lib/resource_quota/resource_quota.h"
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/transport/metadata_batch.h"
#include "src/core/lib/transport/transport.h"

// gmock matcher to ensure that metadata has a key/value pair.
MATCHER_P2(HasMetadataKeyValue, key, value, "") {
  std::string temp;
  auto r = arg.GetStringValue(key, &temp);
  return r == value;
}

// gmock matcher to ensure that a message has a given set of flags.
MATCHER_P(HasMessageFlags, value, "") { return arg.flags() == value; }

// gmock matcher to ensure that a message has a given payload.
MATCHER_P(HasMessagePayload, value, "") {
  return arg.payload()->JoinIntoString() == value;
}

namespace grpc_core {

inline std::ostream& operator<<(std::ostream& os,
                                const grpc_metadata_batch& md) {
  return os << md.DebugString();
}

inline std::ostream& operator<<(std::ostream& os, const Message& msg) {
  return os << "flags:" << msg.flags()
            << " payload:" << absl::CEscape(msg.payload()->JoinIntoString());
}

class FilterTestBase : public ::testing::Test {
 public:
  class Call;

  class Channel {
   private:
    struct Impl {
      explicit Impl(std::unique_ptr<ChannelFilter> filter)
          : filter(std::move(filter)) {}
      size_t initial_arena_size = 1024;
      MemoryAllocator memory_allocator =
          ResourceQuota::Default()->memory_quota()->CreateMemoryAllocator(
              "test");
      std::unique_ptr<ChannelFilter> filter;
    };

   public:
    void set_initial_arena_size(size_t size) {
      impl_->initial_arena_size = size;
    }

    Call MakeCall();

   private:
    friend class FilterTestBase;
    friend class Call;

    explicit Channel(std::unique_ptr<ChannelFilter> filter)
        : impl_(std::make_shared<Impl>(std::move(filter))) {}

    std::shared_ptr<Impl> impl_;
  };

  // One "call" outstanding against this filter.
  // In reality - this filter is the only thing in the call.
  // Provides mocks to trap events that happen on the call.
  class Call {
   public:
    ~Call();
    explicit Call(const Channel& channel);

    // Construct client metadata in the arena of this call.
    // Optional argument is a list of key/value pairs to add to the metadata.
    ClientMetadataHandle NewClientMetadata(
        std::initializer_list<std::pair<absl::string_view, absl::string_view>>
            init = {});
    // Construct server metadata in the arena of this call.
    // Optional argument is a list of key/value pairs to add to the metadata.
    ServerMetadataHandle NewServerMetadata(
        std::initializer_list<std::pair<absl::string_view, absl::string_view>>
            init = {});
    // Construct a message in the arena of this call.
    MessageHandle NewMessage(absl::string_view payload = "",
                             uint32_t flags = 0);

    // Start the call.
    void Start(ClientMetadataHandle md);
    // Cancel the call.
    void Cancel();
    // Forward server initial metadata through this filter.
    void ForwardServerInitialMetadata(ServerMetadataHandle md);
    // Forward a message from client to server through this filter.
    void ForwardMessageClientToServer(MessageHandle msg);
    // Forward a message from server to client through this filter.
    void ForwardMessageServerToClient(MessageHandle msg);
    // Have the 'next' filter in the chain finish this call and return trailing
    // metadata.
    void FinishNextFilter(ServerMetadataHandle md);

    void Step();

    // Mock to trap starting the next filter in the chain.
    MOCK_METHOD(void, Started, (const ClientMetadata& client_initial_metadata));
    // Mock to trap receiving server initial metadata in the next filter in the
    // chain.
    MOCK_METHOD(void, ForwardedServerInitialMetadata,
                (const ServerMetadata& server_initial_metadata));
    // Mock to trap seeing a message forward from client to server.
    MOCK_METHOD(void, ForwardedMessageClientToServer, (const Message& msg));
    // Mock to trap seeing a message forward from server to client.
    MOCK_METHOD(void, ForwardedMessageServerToClient, (const Message& msg));
    // Mock to trap seeing a call finish in the next filter in the chain.
    MOCK_METHOD(void, Finished,
                (const ServerMetadata& server_trailing_metadata));

   private:
    friend class Channel;
    class ScopedContext;
    class Impl;

    std::unique_ptr<Impl> impl_;
  };

 protected:
  absl::StatusOr<Channel> MakeChannel(std::unique_ptr<ChannelFilter> filter) {
    return Channel(std::move(filter));
  }
};

template <typename Filter>
class FilterTest : public FilterTestBase {
 public:
  absl::StatusOr<Channel> MakeChannel(const ChannelArgs& args) {
    auto filter = Filter::Create(args, ChannelFilter::Args());
    if (!filter.ok()) return filter.status();
    return FilterTestBase::MakeChannel(
        std::make_unique<Filter>(std::move(*filter)));
  }
};

}  // namespace grpc_core

#endif  // GRPC_TEST_CORE_FILTERS_FILTER_TEST_H
