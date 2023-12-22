# Copyright 2023 gRPC authors.
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

load("//bazel:grpc_build_system.bzl", "grpc_cc_library", "grpc_cc_test", "grpc_package", "grpc_proto_library")
load("//test/core/util:grpc_fuzzer.bzl", "grpc_fuzzer", "grpc_proto_fuzzer")
load("grpc_transport_test.bzl", "grpc_transport_test")

grpc_cc_library(
    name = "fixture",
    testonly = 1,
    hdrs = ["fixture.h"],
    srcs = ["fixture.cc"],
    deps = ["//:grpc"],
)

grpc_cc_library(
    name = "inproc_fixture",
    testonly = 1,
    srcs = ["inproc_fixture.cc"],
    deps = [
        "fixture",
        "//src/core:grpc_transport_inproc",
    ],
    alwayslink = 1,
)

grpc_cc_library(
    name = "test",
    testonly = 1,
    srcs = ["test.cc"],
    hdrs = ["test.h"],
    external_deps = [
        "absl/functional:any_invocable",
        "absl/strings",
        "gtest",
    ],
    deps = [
        "fixture",
        "//:iomgr_timer",
        "//src/core:resource_quota",
        "//test/core/event_engine/fuzzing_event_engine",
        "//test/core/event_engine/fuzzing_event_engine:fuzzing_event_engine_proto",
    ],
)

grpc_cc_library(
    name = "test_main",
    testonly = 1,
    srcs = ["test_main.cc"],
    deps = [
        "fixture",
        "test",
        "//test/core/util:grpc_test_util",
    ],
)

grpc_transport_test(
    name = "basic_tests",
    deps = [
        ":inproc_fixture",
    ],
)
