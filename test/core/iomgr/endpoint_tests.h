/*
 *
 * Copyright 2015, Google Inc.
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

#ifndef GRPC_TEST_CORE_IOMGR_ENDPOINT_TESTS_H
#define GRPC_TEST_CORE_IOMGR_ENDPOINT_TESTS_H

#include <sys/types.h>

#include "src/core/iomgr/endpoint.h"

typedef struct grpc_endpoint_test_config grpc_endpoint_test_config;
typedef struct grpc_endpoint_test_fixture grpc_endpoint_test_fixture;

struct grpc_endpoint_test_fixture
{
  grpc_endpoint *client_ep;
  grpc_endpoint *server_ep;
};

struct grpc_endpoint_test_config
{
  const char *name;
    grpc_endpoint_test_fixture (*create_fixture) (size_t slice_size);
  void (*clean_up) ();
};

void grpc_endpoint_tests (grpc_endpoint_test_config config,
			  grpc_pollset * pollset);

#endif /* GRPC_TEST_CORE_IOMGR_ENDPOINT_TESTS_H */
