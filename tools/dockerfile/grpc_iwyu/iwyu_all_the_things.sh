#!/bin/sh
# Copyright 2021 gRPC authors.
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

cd ${IWYU_ROOT}

export PATH=/iwyu/build/bin:$PATH

# generate a clang compilation database for all C/C++ sources in the repo.
tools/distrib/gen_compilation_database.py \
  --include_headers \
  --ignore_system_headers \
  --dedup_targets \
  "//:*" \
  "//src/core/..." \
  "//test/core/..."

echo > iwyu.out

cat compile_commands.json | jq '(.. | .directory?) |= "/local-code"' > iwyu_compile_commands.json

# run clang tidy for all source files
cat compile_commands.json | jq -r '.[].file' \
  | grep -E "(^include/|^src/core/|^src/cpp/|^test/core/|^test/cpp/)" \
  | grep -v -E "/upb-generated/|/upbdefs-generated/" \
  | sort \
  | xargs -t python /iwyu/include-what-you-use/iwyu_tool.py -p iwyu_compile_commands.json

#python /iwyu/include-what-you-use/fix_includes.py --nocomment < iwyu.out

