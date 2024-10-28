#!/bin/sh

# Copyright 2015 gRPC authors.
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

set -e

export TEST=true

cd "$(dirname "$0")/../../.."

submodules=$(mktemp /tmp/submXXXXXX)
want_submodules=$(mktemp /tmp/submXXXXXX)

git submodule | sed 's/+//g' | awk '{ print $2 " " $1 }' | sort >"$submodules"
cat <<EOF | sort >"$want_submodules"
third_party/abseil-cpp 4447c7562e3bc702ade25105912dce503f0c4010
third_party/benchmark 344117638c8ff7e239044fd0fa7085839fc03021
third_party/bloaty 60209eb1ccc34d5deefb002d1b7f37545204f7f2
third_party/boringssl-with-bazel b8b3e6e11166719a8ebfa43c0cde9ad7d57a84f6
third_party/cares/cares 6360e96b5cf8e5980c887ce58ef727e53d77243a
third_party/envoy-api f8b75d1efa92bbf534596a013d9ca5873f79dd30
third_party/googleapis fe8ba054ad4f7eca946c2d14a63c3f07c0b586a0
third_party/googletest 2dd1c131950043a8ad5ab0d2dda0e0970596586a
third_party/opencensus-proto 4aa53e15cbf1a47bc9087e6cfdca214c1eea4e89
third_party/opentelemetry 60fa8754d890b5c55949a8c68dcfd7ab5c2395df
third_party/opentelemetry-cpp 4bd64c9a336fd438d6c4c9dad2e6b61b0585311f
third_party/protobuf 10ef3f77683f77fb3c059bf47725c27b3ff41e63
third_party/protoc-gen-validate 32c2415389a3538082507ae537e7edd9578c64ed
third_party/re2 0c5616df9c0aaa44c9440d87422012423d91c7d1
third_party/xds 3a472e524827f72d1ad621c4983dd5af54c46776
third_party/zlib 09155eaa2f9270dc4ed1fa13e2b4b2613e6e4851
EOF

if ! diff -u "$submodules" "$want_submodules"; then
  if [ "$1" = "--fix" ]; then
    while read -r path commit; do
      git submodule update --init "$path"
      (cd "$path" && git checkout "$commit")
    done <"$want_submodules"
    exit 0
  fi
  echo "Submodules are out of sync. Please update this script or run with --fix."
  exit 1
fi

rm "$submodules" "$want_submodules"
