// Copyright 2022 gRPC authors.
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

#include "src/core/lib/experiments/config.h"

#include "absl/strings/str_split.h"

#include <grpc/support/log.h>

#include "src/core/lib/experiments/config.h"
#include "src/core/lib/experiments/experiments.h"
#include "src/core/lib/gprpp/global_config.h"
#include "src/core/lib/gprpp/no_destruct.h"

GPR_GLOBAL_CONFIG_DEFINE_STRING(
    grpc_experiments, "",
    "List of grpc experiments to enable (or with a '-' prefix to disable).");

namespace grpc_core {

namespace {
struct Experiments {
  bool enabled[kNumExperiments];
};
}  // namespace

bool IsExperimentEnabled(size_t experiment_id) {
  // One time initialization:
  static const NoDestruct<Experiments> experiments{[] {
    // Set defaults from metadata.
    Experiments experiments;
    for (size_t i = 0; i < kNumExperiments; i++) {
      experiments.enabled[i] = g_experiment_metadata[i].default_value;
    }
    // Get the global config.
    auto experiments_str = GPR_GLOBAL_CONFIG_GET(grpc_experiments);
    // For each comma-separated experiment in the global config:
    for (auto experiment :
         absl::StrSplit(absl::string_view(experiments_str.get()), ',')) {
      // Strip whitespace.
      experiment = absl::StripAsciiWhitespace(experiment);
      // Handle ",," without crashing.
      if (experiment.empty()) continue;
      // Enable unless prefixed with '-' (=> disable).
      bool enable = true;
      if (experiment[0] == '-') {
        enable = false;
        experiment.remove_prefix(1);
      }
      // See if we can find the experiment in the list in this binary.
      bool found = false;
      for (size_t i = 0; i < kNumExperiments; i++) {
        if (experiment == g_experiment_metadata[i].name) {
          experiments.enabled[i] = enable;
          found = true;
          break;
        }
      }
      // If not found log an error, but don't take any other action.
      // Allows us an easy path to disabling experiments.
      if (!found) {
        gpr_log(GPR_ERROR, "Unknown experiment: %s",
                std::string(experiment).c_str());
      }
    }
    return experiments;
  }()};
  // Normal path: just return the value;
  return experiments->enabled[experiment_id];
}

}  // namespace grpc_core
