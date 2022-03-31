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

#include "http_server_filter.h"

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>

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
  if (grpc_core::Slice* grpc_message =
          metadata->get_pointer(grpc_core::GrpcMessageMetadata())) {
    *grpc_message = grpc_core::PercentEncodeSlice(
        std::move(*grpc_message), grpc_core::PercentEncodingType::Compatible);
  }
}
}  // namespace

ArenaPromise<ServerMetadataHandle> HttpServerFilter::MakeCallPromise(
    CallArgs call_args, NextPromiseFactory next_promise_factory) {
  auto* md = call_args.client_initial_metadata.get();
  auto method = md->get(grpc_core::HttpMethodMetadata());
  if (method.has_value()) {
    switch (*method) {
      case grpc_core::HttpMethodMetadata::kPost:
        break;
      case grpc_core::HttpMethodMetadata::kInvalid:
      case grpc_core::HttpMethodMetadata::kGet:
        return Immediate(
            ServerMetadataHandle(absl::UnknownError("Bad method header")));
    }
  } else {
    return Immediate(
        ServerMetadataHandle(absl::UnknownError("Missing method header")));
  }

  auto te = md->Take(grpc_core::TeMetadata());
  if (te == grpc_core::TeMetadata::kTrailers) {
    // Do nothing, ok.
  } else if (!te.has_value()) {
    return Immediate(
        ServerMetadataHandle(absl::UnknownError("Missing te header")));
  } else {
    return Immediate(ServerMetadataHandle(absl::UnknownError("Bad te header")));
  }

  auto scheme = md->Take(grpc_core::HttpSchemeMetadata());
  if (scheme.has_value()) {
    if (*scheme == grpc_core::HttpSchemeMetadata::kInvalid) {
      return Immediate(
          ServerMetadataHandle(absl::UnknownError("Bad scheme header")));
    }
  } else {
    return Immediate(
        ServerMetadataHandle(absl::UnknownError("Missing scheme header")));
  }

  md->Remove(grpc_core::ContentTypeMetadata());

  grpc_core::Slice* path_slice = md->get_pointer(grpc_core::HttpPathMetadata());
  if (path_slice == nullptr) {
    return Immediate(
        ServerMetadataHandle(absl::UnknownError("Missing path header")));
  }

  if (md->get_pointer(grpc_core::HttpAuthorityMetadata()) == nullptr) {
    absl::optional<grpc_core::Slice> host = md->Take(grpc_core::HostMetadata());
    if (host.has_value()) {
      md->Set(grpc_core::HttpAuthorityMetadata(), std::move(*host));
    }
  }

  if (md->get_pointer(grpc_core::HttpAuthorityMetadata()) == nullptr) {
    return Immediate(
        ServerMetadataHandle(absl::UnknownError("Missing authority header")));
  }

  if (!surface_user_agent_) {
    md->Remove(grpc_core::UserAgentMetadata());
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
            (*md)->Set(grpc_core::HttpStatusMetadata(), 200);
            (*md)->Set(grpc_core::ContentTypeMetadata(),
                       grpc_core::ContentTypeMetadata::kApplicationGrpc);
            FilterOutgoingMetadata(*md);
            write_latch->Set(*md);
            return absl::OkStatus();
          }),
      []() { return absl::OkStatus(); });
}

}  // namespace grpc_core
