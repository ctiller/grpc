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

#ifndef GRPC_CORE_EXT_CLIENT_CONFIG_RETRY_POLICY_H
#define GRPC_CORE_EXT_CLIENT_CONFIG_RETRY_POLICY_H

#include "src/core/ext/client_config/subchannel.h"
#include "src/core/lib/transport/connectivity_state.h"

/** A load balancing policy: specified by a vtable and a struct (which
    is expected to be extended to contain some parameters) */
typedef struct grpc_retry_policy grpc_retry_policy;
typedef struct grpc_retry_policy_vtable grpc_retry_policy_vtable;

#define GRPC_MAX_RETRY_ATTEMPTS 3

struct grpc_retry_policy {
  const grpc_retry_policy_vtable *vtable;
};

struct grpc_retry_attempt {
  grpc_connected_subchannel *picked_channel;
};

struct grpc_request_retry_state {
  gpr_atm selected_attempt;

  int attempt_count;
  grpc_retry_attempt attempts[GRPC_MAX_RETRY_ATTEMPTS];
};

struct grpc_retry_status_update {};

struct grpc_retry_policy_vtable {
  void (*begin_request)(grpc_retry_policy *retry_policy,
                        grpc_request_retry_state *state);
  void (*status_update)(grpc_retry_policy *retry_policy,
                        grpc_request_retry_state *state,
                        grpc_retry_status_update *update);
};

/* API for retry policies */
void grpc_retry_policy_begin_attempt(grpc_retry_policy *policy,
                                     grpc_request_retry_state *state);
void grpc_retry_policy_choose_attempt(grpc_retry_policy *policy,
                                      grpc_request_retry_state *state);

/* API for clients of retry policies */

#endif /* GRPC_CORE_EXT_CLIENT_CONFIG_RETRY_POLICY_H */
