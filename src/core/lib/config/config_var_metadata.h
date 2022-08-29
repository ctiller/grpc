/*
 * Copyright 2022 gRPC authors.
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
 */

/*
 * Automatically generated by tools/codegen/core/gen_config_vars.py
 */

#ifndef GRPC_CORE_LIB_CONFIG_CONFIG_VAR_METADATA_H
#define GRPC_CORE_LIB_CONFIG_CONFIG_VAR_METADATA_H

#include "absl/strings/string_view.h"
#include "absl/types/variant.h"

namespace grpc_core {

class ConfigVars;

struct ConfigVarMetadata {
  template <typename T>
  struct TypeSpecificInfo {
    T default_value;
    T (ConfigVars::*configured_value)();
  };
  using Bool = TypeSpecificInfo<bool>;
  using String = TypeSpecificInfo<absl::string_view>;
  using Int = TypeSpecificInfo<int32_t>;
  using Types = absl::variant<Bool, String, Int>;

  absl::string_view name;
  absl::string_view description;
  bool experiment;

  Types type_specific_info;
};

}  // namespace grpc_core

#endif
