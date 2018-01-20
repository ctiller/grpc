/*

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

/*
  A command line tool to talk to a grpc server.
  Run `grpc_cli help` command to see its usage information.

  Example of talking to grpc interop server:
  grpc_cli call localhost:50051 UnaryCall "response_size:10" \
      --protofiles=src/proto/grpc/testing/test.proto --enable_ssl=false

  Options:
    1. --protofiles, use this flag to provide proto files if the server does
       does not have the reflection service.
    2. --proto_path, if your proto file is not under current working directory,
       use this flag to provide a search root. It should work similar to the
       counterpart in protoc. This option is valid only when protofiles is
       provided.
    3. --metadata specifies metadata to be sent to the server, such as:
       --metadata="MyHeaderKey1:Value1:MyHeaderKey2:Value2"
    4. --enable_ssl, whether to use tls.
    5. --use_auth, if set to true, attach a GoogleDefaultCredentials to the call
    6. --infile, input filename (defaults to stdin)
    7. --outfile, output filename (defaults to stdout)
    8. --binary_input, use the serialized request as input. The serialized
       request can be generated by calling something like:
       protoc --proto_path=src/proto/grpc/testing/ \
         --encode=grpc.testing.SimpleRequest \
         src/proto/grpc/testing/messages.proto \
         < input.txt > input.bin
       If this is used and no proto file is provided in the argument list, the
       method string has to be exact in the form of /package.service/method.
    9. --binary_output, use binary format response as output, it can
       be later decoded using protoc:
       protoc --proto_path=src/proto/grpc/testing/ \
       --decode=grpc.testing.SimpleResponse \
       src/proto/grpc/testing/messages.proto \
       < output.bin > output.txt
*/

#include <fstream>
#include <functional>
#include <iostream>

#include <gflags/gflags.h>
#include <grpc++/support/config.h>
#include "test/cpp/util/cli_credentials.h"
#include "test/cpp/util/grpc_tool.h"
#include "test/cpp/util/test_config.h"

DEFINE_string (outfile, "", "Output file (default is stdout)");

static bool
SimplePrint (const grpc::string & outfile, const grpc::string & output)
{
  if (outfile.empty ())
    {
      std::cout << output << std::endl;
    }
  else
    {
      std::ofstream output_file (outfile, std::ios::app | std::ios::binary);
      output_file << output << std::endl;
      output_file.close ();
    }
  return true;
}

int
main (int argc, char **argv)
{
  grpc::testing::InitTest (&argc, &argv, true);

  return grpc::testing::GrpcToolMainLib (argc, (const char **) argv,
					 grpc::testing::CliCredentials (),
					 std::bind (SimplePrint,
						    FLAGS_outfile,
						    std::placeholders::_1));
}
