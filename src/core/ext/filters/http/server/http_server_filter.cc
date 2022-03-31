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

#include <grpc/support/port_platform.h>

#include "src/core/ext/filters/http/server/http_server_filter.h"

#include <string.h>

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>

#include "src/core/ext/filters/http/server/http_server_filter.h"
#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/gprpp/manual_constructor.h"
#include "src/core/lib/profiling/timers.h"
#include "src/core/lib/promise/call_push_pull.h"
#include "src/core/lib/promise/seq.h"
#include "src/core/lib/slice/b64.h"
#include "src/core/lib/slice/percent_encoding.h"
#include "src/core/lib/slice/slice_internal.h"
#include "src/core/lib/slice/slice_string_helpers.h"

namespace grpc_core {

const grpc_channel_filter HttpServerFilter::kFilter =
    MakePromiseBasedFilter<HttpServerFilter, FilterEndpoint::kServer,
                           kFilterExaminesServerInitialMetadata>("http-server");

absl::StatusOr<HttpServerFilter> HttpServerFilter::Create(ChannelArgs args,
                                                          ChannelFilter::Args) {
  return HttpServerFilter(
      args.GetBool(GRPC_ARG_SURFACE_USER_AGENT).value_or(true));
}

namespace {
void FilterOutgoingMetadata(ServerMetadata* metadata) {
  if (Slice* grpc_message =
          metadata->get_pointer(GrpcMessageMetadata())) {
    *grpc_message = PercentEncodeSlice(
        std::move(*grpc_message), PercentEncodingType::Compatible);
  }
}
}  // namespace

ArenaPromise<ServerMetadataHandle> HttpServerFilter::MakeCallPromise(
    CallArgs call_args, NextPromiseFactory next_promise_factory) {
  auto* md = call_args.client_initial_metadata.get();
  auto method = md->get(HttpMethodMetadata());
  if (method.has_value()) {
    switch (*method) {
      case HttpMethodMetadata::kPost:
        break;
      case HttpMethodMetadata::kInvalid:
      case HttpMethodMetadata::kGet:
        return Immediate(
            ServerMetadataHandle(absl::UnknownError("Bad method header")));
    }
  } else {
    return Immediate(
        ServerMetadataHandle(absl::UnknownError("Missing method header")));
  }

  auto te = md->Take(TeMetadata());
  if (te == TeMetadata::kTrailers) {
    // Do nothing, ok.
  } else if (!te.has_value()) {
    return Immediate(
        ServerMetadataHandle(absl::UnknownError("Missing te header")));
  } else {
    return Immediate(ServerMetadataHandle(absl::UnknownError("Bad te header")));
  }

  auto scheme = md->Take(HttpSchemeMetadata());
  if (scheme.has_value()) {
    if (*scheme == HttpSchemeMetadata::kInvalid) {
      return Immediate(
          ServerMetadataHandle(absl::UnknownError("Bad scheme header")));
    }
  } else {
    return Immediate(
        ServerMetadataHandle(absl::UnknownError("Missing scheme header")));
  }

  md->Remove(ContentTypeMetadata());

  Slice* path_slice = md->get_pointer(HttpPathMetadata());
  if (path_slice == nullptr) {
    return Immediate(
        ServerMetadataHandle(absl::UnknownError("Missing path header")));
  }

  if (md->get_pointer(HttpAuthorityMetadata()) == nullptr) {
    absl::optional<Slice> host = md->Take(HostMetadata());
    if (host.has_value()) {
      md->Set(HttpAuthorityMetadata(), std::move(*host));
    }
  }

  if (md->get_pointer(HttpAuthorityMetadata()) == nullptr) {
    return Immediate(
        ServerMetadataHandle(absl::UnknownError("Missing authority header")));
  }

  if (!surface_user_agent_) {
    md->Remove(UserAgentMetadata());
  }

  auto* read_latch = GetContext<Arena>()->New<Latch<ServerMetadata*>>();
  auto* write_latch =
      absl::exchange(call_args.server_initial_metadata, read_latch);

  return CallPushPull(
      Seq(next_promise_factory(std::move(call_args)),
          [](ServerMetadataHandle trailing_metadata) {
            FilterOutgoingMetadata(trailing_metadata.get());
            return trailing_metadata;
          }),
      Seq(read_latch->Wait(),
          [write_latch](ServerMetadata** md) -> absl::Status {
            (*md)->Set(HttpStatusMetadata(), 200);
            (*md)->Set(ContentTypeMetadata(),
                       ContentTypeMetadata::kApplicationGrpc);
            FilterOutgoingMetadata(*md);
            write_latch->Set(*md);
            return absl::OkStatus();
          }),
      []() { return absl::OkStatus(); });
}

}  // namespace grpc_core
