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

#include "src/core/lib/iomgr/buffer_pool.h"

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>

#include "src/core/lib/iomgr/combiner.h"

struct grpc_buffer_pool {
  grpc_combiner *combiner;

  size_t remaining;
  bool freecycling;

  grpc_buffer_user *roots[GRPC_BUFFER_USER_COUNT];
};

typedef struct {
  gpr_slice_refcount rc;
  gpr_refcount refs;
  size_t in_memory_size;
  grpc_closure free_slice;
  grpc_buffer_user *buffer_user;
} buffer_pool_slice_refcount;

static void queue_alloc(grpc_exec_ctx *exec_ctx, void *bu, grpc_error *error);
static void free_slice(grpc_exec_ctx *exec_ctx, void *bu, grpc_error *error);

static void maybe_fulfill_next(grpc_exec_ctx *exec_ctx, grpc_buffer_pool *pool);

/*******************************************************************************
 * list manipulation
 */

static grpc_buffer_user *list_head(grpc_buffer_pool *buffer_pool,
                                   grpc_buffer_user_list list) {
  return buffer_pool->roots[list];
}

static bool list_empty(grpc_buffer_pool *buffer_pool,
                       grpc_buffer_user_list list) {
  return list_head(buffer_pool, list) == NULL;
}

static void list_remove(grpc_buffer_user *buffer_user,
                        grpc_buffer_user_list list) {
  grpc_buffer_pool *buffer_pool = buffer_user->buffer_pool;
  if (buffer_user == buffer_pool->roots[list]) {
    buffer_pool->roots[list] = buffer_pool->roots[list]->links[list].next;
    if (buffer_user == buffer_pool->roots[list]) {
      buffer_pool->roots[list] = NULL;
    }
  }
  buffer_user->links[list].next->links[list].prev =
      buffer_user->links[list].prev;
  buffer_user->links[list].prev->links[list].next =
      buffer_user->links[list].next;
  buffer_user->links[list].next = buffer_user->links[list].prev = NULL;
}

/* returns true if \a buffer_user is the first element of the list */
static bool list_append(grpc_buffer_user *buffer_user,
                        grpc_buffer_user_list list) {
  grpc_buffer_pool *buffer_pool = buffer_user->buffer_pool;
  if (buffer_pool->roots[list] == NULL) {
    buffer_pool->roots[list] = buffer_user->links[list].next =
        buffer_user->links[list].prev = buffer_user;
    return true;
  }
  buffer_user->links[list].next = buffer_pool->roots[list];
  buffer_user->links[list].prev = buffer_pool->roots[list]->links[list].prev;
  buffer_user->links[list].next->links[list].prev =
      buffer_user->links[list].prev->links[list].next = buffer_user;
  return false;
}

/*******************************************************************************
 * slice implementation
 */

static void slice_ref(void *a) {
  buffer_pool_slice_refcount *rc = a;
  gpr_ref(&rc->refs);
}

static void slice_unref(void *a) {
  buffer_pool_slice_refcount *rc = a;
  if (gpr_unref(&rc->refs)) {
    grpc_buffer_user *buffer_user = rc->buffer_user;
    /* TODO(ctiller): we can't guarantee that it's safe to run an execution
       context here, but it seems we need to. Find Another Way or rewrite rules
       to make it safe. */
    grpc_exec_ctx exec_ctx = GRPC_EXEC_CTX_INIT;
    grpc_combiner_execute(&exec_ctx, buffer_user->buffer_pool->combiner,
                          &rc->free_slice, GRPC_ERROR_NONE, false);
    grpc_exec_ctx_finish(&exec_ctx);
  }
}

static size_t memory_for_slice(size_t size) {
  return sizeof(buffer_pool_slice_refcount) + size;
}

static gpr_slice slice_create(size_t size, grpc_buffer_user *buffer_user) {
  size_t in_memory_size = memory_for_slice(size);
  buffer_pool_slice_refcount *rc = gpr_malloc(in_memory_size);
  rc->rc.ref = slice_ref;
  rc->rc.unref = slice_unref;
  gpr_ref_init(&rc->refs, 1);
  rc->in_memory_size = in_memory_size;
  rc->buffer_user = buffer_user;
  grpc_closure_init(&rc->free_slice, free_slice, rc);

  gpr_slice slice;
  slice.refcount = &rc->rc;
  slice.data.refcounted.bytes = (uint8_t *)(rc + 1);
  slice.data.refcounted.length = size;
  return slice;
}

static void free_slice(grpc_exec_ctx *exec_ctx, void *src, grpc_error *error) {
  buffer_pool_slice_refcount *rc = src;
  grpc_buffer_user *buffer_user = rc->buffer_user;
  grpc_buffer_pool *buffer_pool = buffer_user->buffer_pool;
  buffer_pool->remaining += rc->in_memory_size;
  gpr_free(rc);
  maybe_fulfill_next(exec_ctx, buffer_pool);
}

/*******************************************************************************
 * freecycling
 */

static void freecycling_next(grpc_exec_ctx *exec_ctx,
                             grpc_buffer_pool *buffer_pool) {
  if (list_empty(buffer_pool, GRPC_BUFFER_USER_PENDING_ALLOC)) {
    buffer_pool->freecycling = false;
    return;
  }
  grpc_buffer_user *lru = list_head(buffer_pool, GRPC_BUFFER_USER_ALL);
  GPR_ASSERT(!lru->freecycling);
  list_remove(lru, GRPC_BUFFER_USER_ALL);
  list_append(lru, GRPC_BUFFER_USER_ALL);
}

/*******************************************************************************
 * grpc_buffer_user implementation
 */

void grpc_buffer_user_init(grpc_buffer_user *buffer_user,
                           grpc_buffer_pool *buffer_pool,
                           grpc_buffer_user_vtable *vtable) {
  buffer_user->vtable = vtable;
  buffer_user->buffer_pool = buffer_pool;
  buffer_user->pending_allocation.on_done = NULL;
  grpc_closure_init(&buffer_user->queue_alloc, queue_alloc, buffer_user);
}

void grpc_buffer_user_alloc(grpc_exec_ctx *exec_ctx,
                            grpc_buffer_user *buffer_user,
                            size_t target_slice_count, size_t target_slice_size,
                            gpr_slice_buffer *dest_slice_buffer,
                            grpc_closure *on_done) {
  GPR_ASSERT(buffer_user->pending_allocation.on_done == NULL);

  size_t cur_count = dest_slice_buffer->count;
  if (cur_count >= target_slice_count) {
    grpc_exec_ctx_sched(exec_ctx, on_done, GRPC_ERROR_NONE, NULL);
    return;
  }

  buffer_user->pending_allocation.target = dest_slice_buffer;
  buffer_user->pending_allocation.allocate_slices =
      target_slice_count - cur_count;
  buffer_user->pending_allocation.allocate_slice_size = target_slice_size;
  buffer_user->pending_allocation.on_done = on_done;

  grpc_combiner_execute(exec_ctx, buffer_user->buffer_pool->combiner,
                        &buffer_user->queue_alloc, GRPC_ERROR_NONE, false);
}

static size_t memory_for_alloc(grpc_buffer_user *buffer_user) {
  return memory_for_slice(buffer_user->pending_allocation.allocate_slice_size) *
         buffer_user->pending_allocation.allocate_slices;
}

static void fulfill(grpc_exec_ctx *exec_ctx, grpc_buffer_user *buffer_user) {
  grpc_buffer_pool *buffer_pool = buffer_user->buffer_pool;
  GPR_ASSERT(memory_for_alloc(buffer_user) <= buffer_pool->remaining);

  for (size_t i = 0; i < buffer_user->pending_allocation.allocate_slices; i++) {
    gpr_slice_buffer_add_indexed(
        buffer_user->pending_allocation.target,
        slice_create(buffer_user->pending_allocation.allocate_slice_size,
                     buffer_user));
  }

  buffer_pool->remaining -= memory_for_alloc(buffer_user);

  grpc_closure *c = buffer_user->pending_allocation.on_done;
  buffer_user->pending_allocation.on_done = NULL;
  grpc_exec_ctx_sched(exec_ctx, c, GRPC_ERROR_NONE,
                      buffer_user->response_workqueue);
}

static void maybe_fulfill_next(grpc_exec_ctx *exec_ctx,
                               grpc_buffer_pool *buffer_pool) {
  grpc_buffer_user *buffer_user =
      list_head(buffer_pool, GRPC_BUFFER_USER_PENDING_ALLOC);
  if (buffer_user == NULL) {
    return;
  }
  if (memory_for_alloc(buffer_user) <= buffer_pool->remaining) {
    list_remove(buffer_user, GRPC_BUFFER_USER_PENDING_ALLOC);
    fulfill(exec_ctx, buffer_user);
    maybe_fulfill_next(exec_ctx, buffer_pool); /* loop */
  } else if (!buffer_pool->freecycling) {
    buffer_pool->freecycling = true;
    freecycling_next(exec_ctx, buffer_pool);
  }
}

static void queue_alloc(grpc_exec_ctx *exec_ctx, void *bu, grpc_error *error) {
  grpc_buffer_user *buffer_user = bu;

  list_remove(buffer_user, GRPC_BUFFER_USER_ALL);
  list_append(buffer_user, GRPC_BUFFER_USER_ALL);

  if (list_append(buffer_user, GRPC_BUFFER_USER_PENDING_ALLOC)) {
    maybe_fulfill_next(exec_ctx, buffer_user->buffer_pool);
  }
}
