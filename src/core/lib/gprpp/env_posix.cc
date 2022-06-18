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

#ifdef GPR_POSIX_ENV

#include <stdlib.h>

#include <grpc/support/log.h>

#include "src/core/lib/gprpp/env.h"

namespace grpc_core {

absl::optional<std::string> EnvGet(absl::string_view name) {
  char* result = getenv(std::string(name).c_str());
  return result == nullptr ? absl::optional<std::string> : result;
}

void EnvSet(absl::string_view name, absl::optional<absl::string_view> value) {
  int res = 0;
  if (value.has_value()) {
    res = setenv(std::string(name).c_str(), std::string(*value).c_str(), 1);
  } else {
    res = unsetenv(std::string(name).c_str());
  }
  GPR_ASSERT(res == 0);
}

}  // namespace grpc_core

#endif /* GPR_POSIX_ENV */
