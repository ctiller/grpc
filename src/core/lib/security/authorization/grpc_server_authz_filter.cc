// Copyright 2021 gRPC authors.
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

#include "src/core/lib/security/authorization/grpc_server_authz_filter.h"

#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/str_join.h"

#include <grpc/support/log.h>
#include <grpc/support/port_platform.h>

#include "src/core/lib/channel/channel_stack.h"
#include "src/core/lib/channel/promise_based_filter.h"
#include "src/core/lib/debug/trace.h"
#include "src/core/lib/promise/promise.h"
#include "src/core/lib/security/authorization/authorization_engine.h"
#include "src/core/lib/security/authorization/evaluate_args.h"
#include "src/core/lib/transport/metadata_batch.h"
#include "src/core/lib/transport/transport.h"

namespace grpc_core {

TraceFlag grpc_authz_trace(false, "grpc_authz_api");

const NoInterceptor GrpcServerAuthzFilter::Call::OnServerInitialMetadata;
const NoInterceptor GrpcServerAuthzFilter::Call::OnServerTrailingMetadata;
const NoInterceptor GrpcServerAuthzFilter::Call::OnClientToServerMessage;
const NoInterceptor GrpcServerAuthzFilter::Call::OnClientToServerHalfClose;
const NoInterceptor GrpcServerAuthzFilter::Call::OnServerToClientMessage;
const NoInterceptor GrpcServerAuthzFilter::Call::OnFinalize;

GrpcServerAuthzFilter::GrpcServerAuthzFilter(
    RefCountedPtr<grpc_auth_context> auth_context, const ChannelArgs& args,
    RefCountedPtr<grpc_authorization_policy_provider> provider)
    : auth_context_(std::move(auth_context)),
      per_channel_evaluate_args_(auth_context_.get(), args),
      provider_(std::move(provider)) {}

absl::StatusOr<std::unique_ptr<GrpcServerAuthzFilter>>
GrpcServerAuthzFilter::Create(const ChannelArgs& args, ChannelFilter::Args) {
  auto* auth_context = args.GetObject<grpc_auth_context>();
  auto* provider = args.GetObject<grpc_authorization_policy_provider>();
  if (provider == nullptr) {
    return absl::InvalidArgumentError("Failed to get authorization provider.");
  }
  return std::make_unique<GrpcServerAuthzFilter>(
      auth_context != nullptr ? auth_context->Ref() : nullptr, args,
      provider->Ref());
}

bool GrpcServerAuthzFilter::IsAuthorized(ClientMetadata& initial_metadata) {
  EvaluateArgs args(&initial_metadata, &per_channel_evaluate_args_);
  if (GRPC_TRACE_FLAG_ENABLED(grpc_authz_trace)) {
    gpr_log(GPR_DEBUG,
            "checking request: url_path=%s, transport_security_type=%s, "
            "uri_sans=[%s], dns_sans=[%s], subject=%s",
            std::string(args.GetPath()).c_str(),
            std::string(args.GetTransportSecurityType()).c_str(),
            absl::StrJoin(args.GetUriSans(), ",").c_str(),
            absl::StrJoin(args.GetDnsSans(), ",").c_str(),
            std::string(args.GetSubject()).c_str());
  }
  grpc_authorization_policy_provider::AuthorizationEngines engines =
      provider_->engines();
  if (engines.deny_engine != nullptr) {
    AuthorizationEngine::Decision decision =
        engines.deny_engine->Evaluate(args);
    if (decision.type == AuthorizationEngine::Decision::Type::kDeny) {
      if (GRPC_TRACE_FLAG_ENABLED(grpc_authz_trace)) {
        gpr_log(GPR_INFO, "chand=%p: request denied by policy %s.", this,
                decision.matching_policy_name.c_str());
      }
      return false;
    }
  }
  if (engines.allow_engine != nullptr) {
    AuthorizationEngine::Decision decision =
        engines.allow_engine->Evaluate(args);
    if (decision.type == AuthorizationEngine::Decision::Type::kAllow) {
      if (GRPC_TRACE_FLAG_ENABLED(grpc_authz_trace)) {
        gpr_log(GPR_DEBUG, "chand=%p: request allowed by policy %s.", this,
                decision.matching_policy_name.c_str());
      }
      return true;
    }
  }
  if (GRPC_TRACE_FLAG_ENABLED(grpc_authz_trace)) {
    gpr_log(GPR_INFO, "chand=%p: request denied, no matching policy found.",
            this);
  }
  return false;
}

absl::Status GrpcServerAuthzFilter::Call::OnClientInitialMetadata(
    ClientMetadata& md, GrpcServerAuthzFilter* filter) {
  if (!filter->IsAuthorized(md)) {
    return absl::PermissionDeniedError("Unauthorized RPC request rejected.");
  }
  return absl::OkStatus();
}

const grpc_channel_filter GrpcServerAuthzFilter::kFilter =
    MakePromiseBasedFilter<GrpcServerAuthzFilter, FilterEndpoint::kServer>(
        "grpc-server-authz");

}  // namespace grpc_core
