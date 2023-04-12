//
//
// Copyright 2015 gRPC authors.
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
//

#include <grpc/support/port_platform.h>

#include "src/core/ext/filters/client_channel/client_channel.h"
#include "src/core/ext/filters/client_channel/client_channel_service_config.h"
#include "src/core/ext/filters/client_channel/retry_service_config.h"
#include "src/core/lib/config/core_configuration.h"
#include "src/core/lib/surface/channel_stack_type.h"

namespace grpc_core {

void BuildClientChannelConfiguration(CoreConfiguration::Builder* builder) {
  internal::ClientChannelServiceConfigParser::Register(builder);
  internal::RetryServiceConfigParser::Register(builder);
  builder->channel_init()
      ->RegisterFilter(GRPC_CLIENT_CHANNEL, &ClientChannel::kFilterVtable)
      .Terminal();
}

}  // namespace grpc_core
