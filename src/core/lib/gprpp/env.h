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

#ifndef GRPC_CORE_LIB_GPR_ENV_H
#define GRPC_CORE_LIB_GPR_ENV_H

#include <grpc/support/port_platform.h>

#include <stdio.h>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

/* Env utility functions */

namespace grpc_core {

/* Gets the environment variable value with the specified name.
   Returns a newly allocated string. It is the responsibility of the caller to
   gpr_free the return value if not NULL (which means that the environment
   variable exists). */
absl::optional<std::string> EnvGet(absl::string_view name);

/* Sets the environment with the specified name to the specified value, or
 * deletes it if value is nullopt. */
void EnvSet(absl::string_view name, absl::optional<absl::string_view> value);

}  // namespace grpc_core

#endif /* GRPC_CORE_LIB_GPR_ENV_H */
