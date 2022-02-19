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

#include <stdio.h>
#include <string.h>

#include "absl/strings/str_cat.h"

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/string_util.h>

#include "src/core/lib/gpr/string.h"
#include "src/core/lib/gprpp/host_port.h"
#include "test/core/util/port.h"
#include "test/core/util/subprocess.h"

int main(int /*argc*/, char** argv) {
  char* me = argv[0];
  char* lslash = strrchr(me, '/');
  char root[1024];
  int port = grpc_pick_unused_port_or_die();
  std::vector<const char*> args;
  int status;
  gpr_subprocess *svr, *cli;
  /* figure out where we are */
  if (lslash) {
    memcpy(root, me, static_cast<size_t>(lslash - me));
    root[lslash - me] = 0;
  } else {
    strcpy(root, ".");
  }
  /* start the server */
  std::string cmd = absl::StrCat(root, "/memory_usage_server",
                                 gpr_subprocess_binary_extension());
  std::string joined = grpc_core::JoinHostPort("::", port);
  args = std::vector<const char*>{cmd.c_str(), "--bind", joined.c_str(),
                                  "--no-secure"};
  svr = gpr_subprocess_create(args.size(), args.data());

  /* start the client */
  cmd = absl::StrCat(root, "/memory_usage_client",
                     gpr_subprocess_binary_extension());
  joined = grpc_core::JoinHostPort("127.0.0.1", port);
  args = std::vector<const char*>{
      cmd.c_str(),      "--target",          joined.c_str(),
      "--warmup=10000", "--benchmark=90000",
  };
  cli = gpr_subprocess_create(args.size(), args.data());

  /* wait for completion */
  printf("waiting for client\n");
  if ((status = gpr_subprocess_join(cli))) {
    printf("client failed with: %d", status);
    gpr_subprocess_destroy(cli);
    gpr_subprocess_destroy(svr);
    return 1;
  }
  gpr_subprocess_destroy(cli);

  gpr_subprocess_interrupt(svr);
  status = gpr_subprocess_join(svr);
  gpr_subprocess_destroy(svr);
  return status == 0 ? 0 : 2;
}
