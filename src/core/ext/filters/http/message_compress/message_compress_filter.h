/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef GRPC_CORE_EXT_FILTERS_HTTP_MESSAGE_COMPRESS_MESSAGE_COMPRESS_FILTER_H
#define GRPC_CORE_EXT_FILTERS_HTTP_MESSAGE_COMPRESS_MESSAGE_COMPRESS_FILTER_H

#include <grpc/support/port_platform.h>

#include "src/core/lib/channel/channel_fwd.h"
#include "src/core/lib/channel/channel_stack.h"
#include "src/core/lib/channel/promise_based_filter.h"

/** Compression filter for outgoing data.
 *
 * See <grpc/compression.h> for the available compression settings.
 *
 * Compression settings may come from:
 *  - Channel configuration, as established at channel creation time.
 *  - The metadata accompanying the outgoing data to be compressed. This is
 *    taken as a request only. We may choose not to honor it. The metadata key
 *    is given by \a GRPC_COMPRESSION_REQUEST_ALGORITHM_MD_KEY.
 *
 * Compression can be disabled for concrete messages (for instance in order to
 * prevent CRIME/BEAST type attacks) by having the GRPC_WRITE_NO_COMPRESS set in
 * the BEGIN_MESSAGE flags.
 *
 * The attempted compression mechanism is added to the resulting initial
 * metadata under the'grpc-encoding' key.
 *
 * If compression is actually performed, BEGIN_MESSAGE's flag is modified to
 * incorporate GRPC_WRITE_INTERNAL_COMPRESS. Otherwise, and regardless of the
 * aforementioned 'grpc-encoding' metadata value, data will pass through
 * uncompressed. */

namespace grpc_core {

class MessageCompressFilter : public ChannelFilter {
 public:
  static const grpc_channel_filter kClientFilter;
  static const grpc_channel_filter kServerFilter;

  static absl::StatusOr<MessageCompressFilter> Create(
      const ChannelArgs& args, ChannelFilter::Args filter_args);

  // Construct a promise for one call.
  ArenaPromise<ServerMetadataHandle> MakeCallPromise(
      CallArgs call_args, NextPromiseFactory next_promise_factory) override;

 private:
  explicit MessageCompressFilter(const ChannelArgs& args);

  // The default, channel-level, compression algorithm.
  grpc_compression_algorithm default_compression_algorithm_;
  // Enabled compression algorithms.
  grpc_core::CompressionAlgorithmSet enabled_compression_algorithms_;
};

}  // namespace grpc_core

#endif /* GRPC_CORE_EXT_FILTERS_HTTP_MESSAGE_COMPRESS_MESSAGE_COMPRESS_FILTER_H \
        */
