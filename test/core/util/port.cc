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

#include "src/core/lib/iomgr/port.h"

#include "test/core/util/test_config.h"

#if defined(GRPC_TEST_PICK_PORT)
#include "test/core/util/port.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <grpc/grpc.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/string_util.h>

#include "src/core/lib/address_utils/sockaddr_utils.h"
#include "src/core/lib/http/httpcli.h"
#include "src/core/lib/iomgr/resolve_address.h"
#include "test/core/util/port_server_client.h"
#endif

#if defined(GRPC_TEST_PICK_PORT)

static int* chosen_ports = nullptr;
static size_t num_chosen_ports = 0;
static grpc_core::Mutex* g_default_port_picker_mu;
static gpr_once g_default_port_picker_init = GPR_ONCE_INIT;

static void init_default_port_picker() {
  g_default_port_picker_mu = new grpc_core::Mutex();
}

static int free_chosen_port_locked(int port) {
  size_t i;
  int found = 0;
  size_t found_at = 0;
  /* Find the port and erase it from the list, then tell the server it can be
     freed. */
  for (i = 0; i < num_chosen_ports; i++) {
    if (chosen_ports[i] == port) {
      GPR_ASSERT(found == 0);
      found = 1;
      found_at = i;
    }
  }
  if (found) {
    chosen_ports[found_at] = chosen_ports[num_chosen_ports - 1];
    num_chosen_ports--;
    grpc_free_port_using_server(port);
  }
  return found;
}

static void free_chosen_ports(void) {
  grpc_core::MutexLock lock(g_default_port_picker_mu);
  size_t i;
  grpc_init();
  for (i = 0; i < num_chosen_ports; i++) {
    grpc_free_port_using_server(chosen_ports[i]);
  }
  grpc_shutdown();
  gpr_free(chosen_ports);
}

static void chose_port_locked(int port) {
  if (chosen_ports == nullptr) {
    atexit(free_chosen_ports);
  }
  num_chosen_ports++;
  chosen_ports = static_cast<int*>(
      gpr_realloc(chosen_ports, sizeof(int) * num_chosen_ports));
  chosen_ports[num_chosen_ports - 1] = port;
}

static int grpc_pick_unused_port_impl(void) {
  gpr_once_init(&g_default_port_picker_init, init_default_port_picker);
  grpc_core::MutexLock lock(g_default_port_picker_mu);
  int port = grpc_pick_port_using_server();
  if (port != 0) {
    chose_port_locked(port);
  }

  return port;
}

static int grpc_pick_unused_port_or_die_impl(void) {
  int port = grpc_pick_unused_port();
  if (port == 0) {
    fprintf(stderr,
            "gRPC tests require a helper port server to allocate ports used \n"
            "during the test.\n\n"
            "This server is not currently running.\n\n"
            "To start it, run tools/run_tests/start_port_server.py\n\n");
    exit(1);
  }
  return port;
}

static void grpc_recycle_unused_port_impl(int port) {
  gpr_once_init(&g_default_port_picker_init, init_default_port_picker);
  grpc_core::MutexLock lock(g_default_port_picker_mu);
  GPR_ASSERT(free_chosen_port_locked(port));
}

static grpc_pick_port_functions g_pick_port_functions = {
    grpc_pick_unused_port_impl, grpc_pick_unused_port_or_die_impl,
    grpc_recycle_unused_port_impl};

int grpc_pick_unused_port(void) {
  return g_pick_port_functions.pick_unused_port_fn();
}

int grpc_pick_unused_port_or_die(void) {
  return g_pick_port_functions.pick_unused_port_or_die_fn();
}

void grpc_recycle_unused_port(int port) {
  g_pick_port_functions.recycle_unused_port_fn(port);
}

void grpc_set_pick_port_functions(grpc_pick_port_functions functions) {
  GPR_ASSERT(functions.pick_unused_port_fn != nullptr);
  GPR_ASSERT(functions.pick_unused_port_or_die_fn != nullptr);
  GPR_ASSERT(functions.recycle_unused_port_fn != nullptr);
  g_pick_port_functions = functions;
}

#endif /* GRPC_TEST_PICK_PORT */