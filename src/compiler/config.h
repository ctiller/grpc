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

#ifndef SRC_COMPILER_CONFIG_H
#define SRC_COMPILER_CONFIG_H

#include <string>

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>

#ifdef GRPC_CUSTOM_STRING
#warning GRPC_CUSTOM_STRING is no longer supported. Please use std::string.
#endif

namespace grpc {

// Using grpc::string and grpc::to_string is discouraged in favor of
// std::string and std::to_string. This is only for legacy code using
// them explictly.
using std::string;     // deprecated
using std::to_string;  // deprecated

namespace protobuf {

using Descriptor = ::google::protobuf::Descriptor;
using FileDescriptor = ::google::protobuf::FileDescriptor;
using FileDescriptorProto = ::google::protobuf::FileDescriptorProto;
using MethodDescriptor = ::google::protobuf::MethodDescriptor;
using ServiceDescriptor = ::google::protobuf::ServiceDescriptor;
using SourceLocation = ::google::protobuf::SourceLocation;

namespace compiler {
using CodeGenerator = ::google::protobuf::compiler::CodeGenerator;
using GeneratorContext = ::google::protobuf::compiler::GeneratorContext;
static inline int PluginMain(int argc, char* argv[],
                             const CodeGenerator* generator) {
  return ::google::protobuf::compiler::PluginMain(argc, argv, generator);
}
static inline void ParseGeneratorParameter(
    const string& parameter, std::vector<std::pair<string, string> >* options) {
  ::google::protobuf::compiler::ParseGeneratorParameter(parameter, options);
}

}  // namespace compiler
namespace io {
using Printer = ::google::protobuf::io::Printer;
using CodedOutputStream = ::google::protobuf::io::CodedOutputStream;
using StringOutputStream = ::google::protobuf::io::StringOutputStream;
using ZeroCopyOutputStream = ::google::protobuf::io::ZeroCopyOutputStream;
}  // namespace io
}  // namespace protobuf
}  // namespace grpc

namespace grpc_cpp_generator {

static const char* const kCppGeneratorMessageHeaderExt = ".pb.h";
static const char* const kCppGeneratorServiceHeaderExt = ".grpc.pb.h";

}  // namespace grpc_cpp_generator

#endif  // SRC_COMPILER_CONFIG_H
