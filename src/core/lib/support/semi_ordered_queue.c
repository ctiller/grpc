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

#include <assert.h>
#include <grpc/support/log.h>
#include <stdlib.h>

typedef union {
  gpr_atm atm;
  struct {
    uint8_t draining;
    uint8_t enqueue_pos : GPR_SEMI_ORDERED_QUEUE_HEADQ_LOG2_SIZE;
    uint8_t dequeue_pos : GPR_SEMI_ORDERED_QUEUE_HEADQ_LOG2_SIZE;
    uint8_t count : (GPR_SEMI_ORDERED_QUEUE_HEADQ_LOG2_SIZE + 1);
  } s;
} state_word;

static state_word load_state_no_barrier(gpr_semi_ordered_queue *q) {
  return (state_word){.atm = gpr_atm_no_barrier_load(&q->state)};
}

static bool cas_state_no_barrier(gpr_semi_ordered_queue *q, state_word cur,
                                 state_word new) {
  return gpr_atm_no_barrier_cas(&q->state, cur.atm, new.atm);
}

static bool push_head(gpr_semi_ordered_queue *q, gpr_mpscq_node *n,
                      state_word *st) {
  while (st->s.count != GPR_SEMI_ORDERED_QUEUE_HEADQ_SIZE) {
    state_word stp = *st;
    stp.s.enqueue_pos++;
    stp.s.count++;
    if (cas_state_no_barrier(q, *st, stp)) {
      assert(gpr_atm_no_barrier_load(&q->headq[st->s.enqueue_pos]) == 0);
      gpr_atm_rel_store(&q->headq[st->s.enqueue_pos], (gpr_atm)n);
      return true;
    }
    *st = load_state_no_barrier(q);
  }
  return false;
}

void gpr_semi_ordered_queue_push(gpr_semi_ordered_queue *q, gpr_mpscq_node *n) {
  gpr_mpscq_push(&q->tailq, n);
}

static bool drain_then_pop(gpr_semi_ordered_queue *q, gpr_mpscq_node **out,
                           state_word st) {
  gpr_mpscq_node *n;
  while (gpr_mpscq_pop(&q->tailq, &n)) {
    if (n == NULL) {
      break;
    }
    while (!push_head(q, n, &st)) {
      state_word stp = st;
      stp.s.draining = 0;
      if (cas_state_no_barrier(q, st, stp)) {
        *out = n;
        return true;
      }
      st = load_state_no_barrier(q);
    }
  }
}

bool gpr_semi_ordered_queue_pop(gpr_semi_ordered_queue *q, gpr_mpscq_node **n) {
  state_word st = load_state_no_barrier(q);
  for (;;) {
    if (st.s.count == 0) {
      if (!st.s.draining) {
        state_word stp = st;
        stp.s.draining = 1;
        if (cas_state_no_barrier(q, st, stp)) {
          return drain_then_pop(q, n, stp);
        }
        st = load_state_no_barrier(q);
        break;
      }
    }
  }
}
