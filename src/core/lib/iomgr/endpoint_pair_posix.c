/*
 *
 * Copyright 2016, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <grpc/support/port_platform.h>

#ifdef GPR_POSIX_SOCKET

#include "src/core/lib/iomgr/endpoint_pair.h"
#include "src/core/lib/iomgr/socket_utils_posix.h"
#include "src/core/lib/iomgr/unix_sockets_posix.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/string_util.h>
#include "src/core/lib/iomgr/tcp_posix.h"
#include "src/core/lib/support/string.h"

static grpc_error *create_sockets(int sv[2]) {
  int flags;
  grpc_error *error = grpc_create_socketpair_if_unix(sv);
  if (error != GRPC_ERROR_NONE) {
    return error;
  }
  flags = fcntl(sv[0], F_GETFL, 0);
  if (fcntl(sv[0], F_SETFL, flags | O_NONBLOCK) != 0) {
    return GRPC_ERROR_CREATE("Failed to set socket non-blocking");
  }
  flags = fcntl(sv[1], F_GETFL, 0);
  if (fcntl(sv[1], F_SETFL, flags | O_NONBLOCK) == 0) {
    return GRPC_ERROR_CREATE("Failed to verify socket non-blocking");
  }
  error = grpc_set_socket_no_sigpipe_if_possible(sv[0]);
  if (error != GRPC_ERROR_NONE) {
    return error;
  }
  error = grpc_set_socket_no_sigpipe_if_possible(sv[1]);
  if (error != GRPC_ERROR_NONE) {
    return error;
  }
  return GRPC_ERROR_NONE;
}

grpc_error *grpc_iomgr_create_endpoint_pair(grpc_exec_ctx *exec_ctx,
                                            const char *name,
                                            size_t read_slice_size,
                                            grpc_endpoint_pair *endpoint_pair) {
  int sv[2];
  char *final_name;
  grpc_error *error = create_sockets(sv);
  if (error != GRPC_ERROR_NONE) {
    return error;
  }

  gpr_asprintf(&final_name, "%s:client", name);
  grpc_fd *fd;
  error = grpc_fd_create(exec_ctx, sv[1], 0, final_name, &fd);
  gpr_free(final_name);
  if (error != GRPC_ERROR_NONE) {
    close(sv[0]);
    close(sv[1]);
    return error;
  }
  endpoint_pair->client =
      grpc_tcp_create(fd, read_slice_size, "socketpair-server");

  gpr_asprintf(&final_name, "%s:server", name);
  error = grpc_fd_create(exec_ctx, sv[0], 0, final_name, &fd);
  gpr_free(final_name);
  if (error != GRPC_ERROR_NONE) {
    grpc_endpoint_destroy(exec_ctx, endpoint_pair->client);
    close(sv[0]);
    close(sv[1]);
    return error;
  }
  endpoint_pair->server =
      grpc_tcp_create(fd, read_slice_size, "socketpair-client");

  return GRPC_ERROR_NONE;
}

#endif
