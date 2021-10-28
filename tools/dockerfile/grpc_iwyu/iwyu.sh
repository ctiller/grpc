#!/bin/sh
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

cd ${IWYU_ROOT}

export PATH=${PATH}:/iwyu_build/bin

cat compile_commands.json | sed "s,\"file\": \",\"file\": \"${IWYU_ROOT}/,g" > compile_commands_for_iwyu.json

# run clang tidy for all source files
cat compile_commands.json | jq -r '.[].file' \
  | grep -E "^src/core/lib/promise/" \
  | grep -v -E "/upb-generated/|/upbdefs-generated/" \
  | sort \
  | tee iwyu_files.txt

xargs -a iwyu_files.txt /iwyu/iwyu_tool.py -p compile_commands_for_iwyu.json -j 16 \
  | grep -v -E "port_platform.h" \
  | tee iwyu.out

/iwyu/fix_includes.py --nocomments < iwyu.out || true

xargs -a iwyu_files.txt clang-format-11 -i

# TODO(ctiller): expand this to match the clang-tidy directories:
#  | grep -E "(^include/|^src/core/|^src/cpp/|^test/core/|^test/cpp/)"
