#!/usr/bin/env bash
# Copyright 2017 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -ex

# avoid slow finalization after the script has exited.
source $(dirname $0)/../../../tools/internal_ci/helper_scripts/move_src_tree_and_respawn_itself_rc

# Enter the gRPC repo root
cd $(dirname $0)/../../..

cat tools/internal_ci/linux/grpc_sample_fuzzers_failure_explanation.txt

source tools/internal_ci/helper_scripts/prepare_build_linux_rc

# make sure bazel is available
tools/bazel version

python3 tools/run_tests/python_utils/bazel_report_helper.py --report_path bazel_rbe

bazel_rbe/bazel_wrapper \
  --bazelrc=tools/remote_build/linux_kokoro.bazelrc \
  test \
  -c dbg --config fuzzer_asan --test_output=errors \
  `tools/bazel query "attr(tags, '\\bgrpc-fuzzer\\b', //test/...)"`
