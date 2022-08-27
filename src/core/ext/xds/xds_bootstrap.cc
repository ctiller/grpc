//
// Copyright 2019 gRPC authors.
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
//

#include <grpc/support/port_platform.h>

#include "src/core/ext/xds/xds_bootstrap.h"

#include <set>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"

#include <grpc/support/alloc.h>

#include "src/core/lib/gpr/env.h"
#include "src/core/lib/gpr/string.h"

namespace grpc_core {

// TODO(donnadionne): check to see if federation is enabled, this will be
// removed once federation is fully integrated and enabled by default.
bool XdsFederationEnabled() {
  char* value = gpr_getenv("GRPC_EXPERIMENTAL_XDS_FEDERATION");
  bool parsed_value;
  bool parse_succeeded = gpr_parse_bool_value(value, &parsed_value);
  gpr_free(value);
  return parse_succeeded && parsed_value;
}

//
// XdsBootstrap::XdsServer
//

constexpr absl::string_view XdsBootstrap::XdsServer::kServerFeatureXdsV3;
constexpr absl::string_view
    XdsBootstrap::XdsServer::kServerFeatureIgnoreResourceDeletion;

bool XdsBootstrap::XdsServer::ShouldUseV3() const {
  return server_features.find(std::string(kServerFeatureXdsV3)) !=
         server_features.end();
}

bool XdsBootstrap::XdsServer::IgnoreResourceDeletion() const {
  return server_features.find(std::string(
             kServerFeatureIgnoreResourceDeletion)) != server_features.end();
}

//
// XdsBootstrap
//

const XdsBootstrap::Authority* XdsBootstrap::LookupAuthority(
    const std::string& name) const {
  auto it = authorities().find(name);
  if (it != authorities().end()) {
    return &it->second;
  }
  return nullptr;
}

bool XdsBootstrap::XdsServerExists(
    const XdsBootstrap::XdsServer& server) const {
  if (server == this->server()) return true;
  for (auto& authority : authorities()) {
    for (auto& xds_server : authority.second.xds_servers) {
      if (server == xds_server) return true;
    }
  }
  return false;
}

}  // namespace grpc_core
