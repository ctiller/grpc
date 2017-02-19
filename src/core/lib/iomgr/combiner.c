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

#include "src/core/lib/iomgr/combiner.h"

#include <string.h>

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>

#include "src/core/lib/iomgr/workqueue.h"
#include "src/core/lib/profiling/timers.h"

int grpc_combiner_trace = 0;

#define GRPC_COMBINER_TRACE(fn) \
  do {                          \
    if (grpc_combiner_trace) {  \
      fn;                       \
    }                           \
  } while (0)

#define EXT_STUB_UNLOCKED ((grpc_closure *)1)
struct grpc_combiner {
  grpc_combiner *next_combiner_on_this_exec_ctx;
  grpc_workqueue *optional_workqueue;
  grpc_closure_scheduler uncovered_scheduler;
  grpc_closure_scheduler covered_scheduler;
  grpc_closure_scheduler uncovered_finally_scheduler;
  grpc_closure_scheduler covered_finally_scheduler;
  grpc_closure offload;
  gpr_refcount refs;

  gpr_atm covers;
  grpc_closure *ext_head;
  gpr_atm ext_tail;
  grpc_closure *exec_head;
  grpc_closure *final_head;
  grpc_closure *final_tail;

  grpc_closure ext_stub_locked;
};

static void combiner_exec_uncovered(grpc_exec_ctx *exec_ctx,
                                    grpc_closure *closure, grpc_error *error);
static void combiner_exec_covered(grpc_exec_ctx *exec_ctx,
                                  grpc_closure *closure, grpc_error *error);
static void combiner_finally_exec_uncovered(grpc_exec_ctx *exec_ctx,
                                            grpc_closure *closure,
                                            grpc_error *error);
static void combiner_finally_exec_covered(grpc_exec_ctx *exec_ctx,
                                          grpc_closure *closure,
                                          grpc_error *error);

static const grpc_closure_scheduler_vtable scheduler_uncovered = {
    combiner_exec_uncovered, combiner_exec_uncovered,
    "combiner:immediately:uncovered"};
static const grpc_closure_scheduler_vtable scheduler_covered = {
    combiner_exec_covered, combiner_exec_covered,
    "combiner:immediately:covered"};
static const grpc_closure_scheduler_vtable finally_scheduler_uncovered = {
    combiner_finally_exec_uncovered, combiner_finally_exec_uncovered,
    "combiner:finally:uncovered"};
static const grpc_closure_scheduler_vtable finally_scheduler_covered = {
    combiner_finally_exec_covered, combiner_finally_exec_covered,
    "combiner:finally:covered"};

static void offload(grpc_exec_ctx *exec_ctx, void *arg, grpc_error *error);

typedef struct {
  grpc_error *error;
  bool covered_by_poller;
} error_data;

static uintptr_t pack_error_data(error_data d) {
  return ((uintptr_t)d.error) | (d.covered_by_poller ? 1 : 0);
}

static error_data unpack_error_data(uintptr_t p) {
  return (error_data){(grpc_error *)(p & ~(uintptr_t)1), p & 1};
}

grpc_combiner *grpc_combiner_create(grpc_workqueue *optional_workqueue) {
  grpc_combiner *lock = gpr_malloc(sizeof(*lock));
  gpr_ref_init(&lock->refs, 1);
  lock->next_combiner_on_this_exec_ctx = NULL;
  lock->optional_workqueue = optional_workqueue;
  lock->uncovered_scheduler.vtable = &scheduler_uncovered;
  lock->covered_scheduler.vtable = &scheduler_covered;
  lock->uncovered_finally_scheduler.vtable = &finally_scheduler_uncovered;
  lock->covered_finally_scheduler.vtable = &finally_scheduler_covered;
  grpc_closure_init(&lock->offload, offload, lock,
                    grpc_workqueue_scheduler(optional_workqueue));

  gpr_atm_no_barrier_store(&lock->covers, 0);
  gpr_atm_rel_store(&lock->ext_tail, EXT_STUB_UNLOCKED);
  lock->ext_head = &lock->ext_stub_locked;
  lock->exec_head = NULL;
  lock->final_head = NULL;
  lock->final_tail = NULL;

  GRPC_COMBINER_TRACE(gpr_log(GPR_DEBUG, "C:%p create", lock));
  return lock;
}

#if 0
static void really_destroy(grpc_exec_ctx *exec_ctx, grpc_combiner *lock) {
  GRPC_COMBINER_TRACE(gpr_log(GPR_DEBUG, "C:%p really_destroy", lock));
  GRPC_WORKQUEUE_UNREF(exec_ctx, lock->optional_workqueue, "combiner");
  gpr_free(lock);
}
#endif

static void start_destroy(grpc_exec_ctx *exec_ctx, grpc_combiner *lock) {}

#ifdef GRPC_COMBINER_REFCOUNT_DEBUG
#define GRPC_COMBINER_DEBUG_SPAM(op, delta)                               \
  gpr_log(file, line, GPR_LOG_SEVERITY_DEBUG,                             \
          "combiner[%p] %s %" PRIdPTR " --> %" PRIdPTR " %s", lock, (op), \
          gpr_atm_no_barrier_load(&lock->refs.count),                     \
          gpr_atm_no_barrier_load(&lock->refs.count) + (delta), reason);
#else
#define GRPC_COMBINER_DEBUG_SPAM(op, delta)
#endif

void grpc_combiner_unref(grpc_exec_ctx *exec_ctx,
                         grpc_combiner *lock GRPC_COMBINER_DEBUG_ARGS) {
  GRPC_COMBINER_DEBUG_SPAM("UNREF", -1);
  if (gpr_unref(&lock->refs)) {
    start_destroy(exec_ctx, lock);
  }
}

grpc_combiner *grpc_combiner_ref(grpc_combiner *lock GRPC_COMBINER_DEBUG_ARGS) {
  GRPC_COMBINER_DEBUG_SPAM("  REF", 1);
  gpr_ref(&lock->refs);
  return lock;
}

static void push_last_on_exec_ctx(grpc_exec_ctx *exec_ctx,
                                  grpc_combiner *lock) {
  lock->next_combiner_on_this_exec_ctx = NULL;
  if (exec_ctx->active_combiner == NULL) {
    exec_ctx->active_combiner = exec_ctx->last_combiner = lock;
  } else {
    exec_ctx->last_combiner->next_combiner_on_this_exec_ctx = lock;
    exec_ctx->last_combiner = lock;
  }
}

#if 0
static void push_first_on_exec_ctx(grpc_exec_ctx *exec_ctx,
                                   grpc_combiner *lock) {
  lock->next_combiner_on_this_exec_ctx = exec_ctx->active_combiner;
  exec_ctx->active_combiner = lock;
  if (lock->next_combiner_on_this_exec_ctx == NULL) {
    exec_ctx->last_combiner = lock;
  }
}
#endif

static void combiner_exec(grpc_exec_ctx *exec_ctx, grpc_combiner *lock,
                          grpc_closure *cl, grpc_error *error,
                          bool covered_by_poller) {
  GPR_TIMER_BEGIN("combiner.execute", 0);
  cl->error_data.scratch = pack_error_data(
      (error_data){.error = error, .covered_by_poller = covered_by_poller});
  gpr_atm_no_barrier_store(&cl->next_data.atm_next.next, (gpr_atm)NULL);
  if (covered_by_poller) gpr_atm_no_barrier_fetch_add(&lock->covers, 1);
  grpc_closure *prev =
      (grpc_closure *)gpr_atm_full_xchg(&lock->ext_tail, (gpr_atm)cl);
  if (prev == EXT_STUB_UNLOCKED) {
    push_last_on_exec_ctx(exec_ctx, lock);
    prev = &lock->ext_stub_locked;
  }
  gpr_atm_rel_store(&prev->next_data.atm_next.next, (gpr_atm)cl);
  GPR_TIMER_END("combiner.execute", 0);
}

#define COMBINER_FROM_CLOSURE_SCHEDULER(closure, scheduler_name) \
  ((grpc_combiner *)(((char *)((closure)->scheduler)) -          \
                     offsetof(grpc_combiner, scheduler_name)))

static void combiner_exec_uncovered(grpc_exec_ctx *exec_ctx, grpc_closure *cl,
                                    grpc_error *error) {
  combiner_exec(exec_ctx,
                COMBINER_FROM_CLOSURE_SCHEDULER(cl, uncovered_scheduler), cl,
                error, false);
}

static void combiner_exec_covered(grpc_exec_ctx *exec_ctx, grpc_closure *cl,
                                  grpc_error *error) {
  combiner_exec(exec_ctx,
                COMBINER_FROM_CLOSURE_SCHEDULER(cl, covered_scheduler), cl,
                error, true);
}

static void move_next(grpc_exec_ctx *exec_ctx) {
  exec_ctx->active_combiner =
      exec_ctx->active_combiner->next_combiner_on_this_exec_ctx;
  if (exec_ctx->active_combiner == NULL) {
    exec_ctx->last_combiner = NULL;
  }
}

static void offload(grpc_exec_ctx *exec_ctx, void *arg, grpc_error *error) {
  grpc_combiner *lock = arg;
  push_last_on_exec_ctx(exec_ctx, lock);
}

static void queue_offload(grpc_exec_ctx *exec_ctx, grpc_combiner *lock) {
  move_next(exec_ctx);
  GRPC_COMBINER_TRACE(gpr_log(GPR_DEBUG, "C:%p queue_offload --> %p", lock,
                              lock->optional_workqueue));
  grpc_closure_sched(exec_ctx, &lock->offload, GRPC_ERROR_NONE);
}

static bool is_covered_by_poller(grpc_combiner *combiner) {
  return combiner->optional_workqueue != NULL &&
         gpr_atm_acq_load(&combiner->covers) > 0;
}

static bool retry_later(grpc_exec_ctx *exec_ctx) {
  grpc_combiner *lock = exec_ctx->active_combiner;
  if (is_covered_by_poller(lock)) {
    queue_offload(exec_ctx, lock);
  } else {
    move_next(exec_ctx);
    push_last_on_exec_ctx(exec_ctx, lock);
  }
  return grpc_combiner_continue_exec_ctx(exec_ctx);
}

bool grpc_combiner_continue_exec_ctx(grpc_exec_ctx *exec_ctx) {
  GPR_TIMER_BEGIN("combiner.continue_exec_ctx", 0);
  grpc_combiner *lock = exec_ctx->active_combiner;
  if (lock == NULL) {
    GPR_TIMER_END("combiner.continue_exec_ctx", 0);
    return false;
  }

  if (lock->optional_workqueue != NULL && is_covered_by_poller(lock) &&
      grpc_exec_ctx_ready_to_finish(exec_ctx)) {
    GPR_TIMER_MARK("offload_from_finished_exec_ctx", 0);
    // this execution context wants to move on, and we have a workqueue (and
    // so can help the execution context out): schedule remaining work to be
    // picked up on the workqueue
    queue_offload(exec_ctx, lock);
    GPR_TIMER_END("combiner.continue_exec_ctx", 0);
    return true;
  }

  grpc_closure *ex = lock->exec_head;
  error_data err;

  if (ex == NULL) {
    for (;;) {
      grpc_closure *head = lock->ext_head;
      grpc_closure *next =
          (grpc_closure *)gpr_atm_acq_load(&head->next_data.atm_next.next);
      if (head == &lock->ext_stub_locked) {
        if (next == NULL) {
          // indicates the external list is (ephemerally) empty
          // attempt to start on the final list
          ex = lock->final_head;
          if (ex != NULL) {
            // start on the final list
            lock->final_head = lock->final_tail = NULL;
            break;  // continue executing
          } else {
            // we've reached the end of the external list and the end of the
            // final list: we can release the combiner
            if (gpr_atm_rel_cas(&lock->ext_tail,
                                (gpr_atm)&lock->ext_stub_locked,
                                (gpr_atm)EXT_STUB_UNLOCKED)) {
              move_next(exec_ctx);
              return false;
            } else {
              continue;  // cas failed: something raced and we have more work to
                         // do
            }
          }
        }
        lock->ext_head = next;
        head = next;
        next = (grpc_closure *)gpr_atm_acq_load(&head->next_data.atm_next.next);
      }
      grpc_closure *tail = (grpc_closure *)gpr_atm_acq_load(&lock->ext_tail);
      if (next != NULL) {
        // found a node, take it
        lock->ext_head = next;
        ex = head;
        ex->next_data.next = NULL;
        break;  // continue executing
      }
      if (head != tail) {
        // there's still an enqueue going on: retry later
        return retry_later(exec_ctx);
      }
      gpr_atm_no_barrier_store(&lock->ext_stub_locked.next_data.atm_next.next,
                               (gpr_atm)NULL);
      grpc_closure *prev = (grpc_closure *)gpr_atm_full_xchg(
          &lock->ext_tail, (gpr_atm)&lock->ext_stub_locked);
      gpr_atm_rel_store(&prev->next_data.atm_next.next,
                        (gpr_atm)&lock->ext_stub_locked);
      next = (grpc_closure *)gpr_atm_acq_load(&head->next_data.atm_next.next);
      if (next != NULL) {
        lock->ext_head = next;
        ex = head;
        ex->next_data.next = NULL;
        break;  // found work, get on with it
      }
      // there's still an enqueue going on: retry later
      return retry_later(exec_ctx);
    }
  }

  err = unpack_error_data(ex->error_data.scratch);
  lock->exec_head = ex->next_data.next;
  ex->cb(exec_ctx, ex->cb_arg, err.error);
  if (err.covered_by_poller) gpr_atm_no_barrier_fetch_add(&lock->covers, -1);
  GRPC_ERROR_UNREF(err.error);

  GPR_TIMER_END("combiner.continue_exec_ctx", 0);
  return true;
}

static void enqueue_finally(grpc_exec_ctx *exec_ctx, void *closure,
                            grpc_error *error);

static void combiner_execute_finally(grpc_exec_ctx *exec_ctx,
                                     grpc_combiner *lock, grpc_closure *closure,
                                     grpc_error *error,
                                     bool covered_by_poller) {
  GRPC_COMBINER_TRACE(gpr_log(
      GPR_DEBUG, "C:%p grpc_combiner_execute_finally c=%p; ac=%p; cov=%d", lock,
      closure, exec_ctx->active_combiner, covered_by_poller));
  GPR_TIMER_BEGIN("combiner.execute_finally", 0);
  if (exec_ctx->active_combiner != lock) {
    GPR_TIMER_MARK("slowpath", 0);
    grpc_closure_sched(
        exec_ctx, grpc_closure_create(enqueue_finally, closure,
                                      grpc_combiner_scheduler(lock, false)),
        error);
    GPR_TIMER_END("combiner.execute_finally", 0);
    return;
  }

  if (lock->final_head == NULL) {
    lock->final_head = closure;
    lock->final_tail = closure;
    closure->next_data.next = NULL;
  } else {
    lock->final_tail->next_data.next = closure;
    lock->final_tail = closure;
    closure->next_data.next = NULL;
  }
  GPR_TIMER_END("combiner.execute_finally", 0);
}

static void enqueue_finally(grpc_exec_ctx *exec_ctx, void *closure,
                            grpc_error *error) {
  combiner_execute_finally(exec_ctx, exec_ctx->active_combiner, closure,
                           GRPC_ERROR_REF(error), false);
}

static void combiner_finally_exec_uncovered(grpc_exec_ctx *exec_ctx,
                                            grpc_closure *cl,
                                            grpc_error *error) {
  combiner_execute_finally(exec_ctx, COMBINER_FROM_CLOSURE_SCHEDULER(
                                         cl, uncovered_finally_scheduler),
                           cl, error, false);
}

static void combiner_finally_exec_covered(grpc_exec_ctx *exec_ctx,
                                          grpc_closure *cl, grpc_error *error) {
  combiner_execute_finally(
      exec_ctx, COMBINER_FROM_CLOSURE_SCHEDULER(cl, covered_finally_scheduler),
      cl, error, true);
}

grpc_closure_scheduler *grpc_combiner_scheduler(grpc_combiner *combiner,
                                                bool covered_by_poller) {
  return covered_by_poller ? &combiner->covered_scheduler
                           : &combiner->uncovered_scheduler;
}

grpc_closure_scheduler *grpc_combiner_finally_scheduler(
    grpc_combiner *combiner, bool covered_by_poller) {
  return covered_by_poller ? &combiner->covered_finally_scheduler
                           : &combiner->uncovered_finally_scheduler;
}
