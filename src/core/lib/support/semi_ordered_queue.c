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

#include "src/core/lib/support/semi_ordered_queue.h"

#include <stdlib.h>

typedef enum {
  PUSH_HEAD_POP_HEAD,
  PUSH_TAIL_POP_HEAD,
  PUSH_TAIL_POP_HEAD_DRAINING,
} state_enum;

typedef union {
  gpr_atm atm;
  struct {
    uint8_t state;
    uint8_t enqueue_pos;
    uint8_t dequeue_pos;
  } s;
} state_word;

static state_word load_state_acq(gpr_semi_ordered_queue *q) {
  return (state_word){.atm = gpr_atm_acq_load(&q->state)};
}

void gpr_semi_ordered_queue_push(gpr_semi_ordered_queue *q, gpr_mpscq_node *n) {
  state_word st = load_state_acq(q);
  switch (st.s.state) {
    case PUSH_TAIL_POP_HEAD:
    case PUSH_TAIL_POP_HEAD_DRAINING:
      gpr_mpscq_push(&q->tailq, n);
      break;
    case PUSH_HEAD_POP_HEAD:
      // mpmcq stuff
      abort();
  }
}

gpr_mpscq_node *gpr_semi_ordered_queue_pop(gpr_semi_ordered_queue *q) {
  state_word st = load_state_acq(q);
}
