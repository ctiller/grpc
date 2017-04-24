/*
 *
 * Copyright 2017, Google Inc.
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

#ifndef GRPC_CORE_LIB_SUPPORT_SEMI_ORDERED_QUEUE_H
#define GRPC_CORE_LIB_SUPPORT_SEMI_ORDERED_QUEUE_H

#include "src/core/lib/support/mpscq.h"

// Multiple-producer multiple-consumer lock free queue
// Guarantees progress, but not strict FIFO ordering (some events may come
// out-of-order)

#define GPR_SEMI_ORDERED_QUEUE_HEADQ_SIZE 256

typedef struct gpr_soq_cell {
  gpr_mpscq_node *node;
  gpr_atm seq;
} gpr_soq_cell;

typedef struct gpr_semi_ordered_queue {
  gpr_atm state;
  gpr_mpscq tailq;
  gpr_mpscq_node headq[GPR_SEMI_ORDERED_QUEUE_HEADQ_SIZE];
} gpr_semi_ordered_queue;

void gpr_semi_ordered_queue_init(gpr_semi_ordered_queue *q);
void gpr_semi_ordered_queue_destroy(gpr_semi_ordered_queue *q);
void gpr_semi_ordered_queue_push(gpr_semi_ordered_queue *q, gpr_mpscq_node *n);
gpr_mpscq_node *gpr_semi_ordered_queue_pop(gpr_semi_ordered_queue *q);

#endif  // GRPC_CORE_LIB_SUPPORT_SEMI_ORDERED_QUEUE_H
