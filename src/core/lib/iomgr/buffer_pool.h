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

#ifndef GRPC_CORE_LIB_IOMGR_BUFFER_POOL_H
#define GRPC_CORE_LIB_IOMGR_BUFFER_POOL_H

#include <grpc/support/slice_buffer.h>

#include "src/core/lib/iomgr/closure.h"
#include "src/core/lib/iomgr/workqueue.h"

typedef struct grpc_buffer_pool grpc_buffer_pool;

grpc_buffer_pool *grpc_buffer_pool_create(void);
void grpc_buffer_pool_resize(size_t size);
grpc_buffer_pool *grpc_buffer_pool_ref(grpc_buffer_pool *bp);
void grpc_buffer_pool_unref(grpc_buffer_pool *bp);

typedef struct grpc_buffer_user grpc_buffer_user;

typedef enum {
  GRPC_BUFFER_RECLAIM_UNUSED_BUFFERS,
  GRPC_BUFFER_RECLAIM_IDLE_STREAMS,
  GRPC_BUFFER_RECLAIM_IDLE_CHANNELS,
  GRPC_BUFFER_RECLAIM_EMERGENCY,
  GRPC_BUFFER_RECLAIM_SHUTDOWN
} grpc_buffer_reclaimation_phase;

typedef void (*grpc_buffer_user_cleanup_function)(
    grpc_exec_ctx *exec_ctx, void *arg, grpc_buffer_reclaimation_phase phase,
    grpc_closure *on_done);

typedef enum {
  GRPC_BUFFER_USER_ALL = 0,
  GRPC_BUFFER_USER_PENDING_ALLOC,
  GRPC_BUFFER_USER_RECLAIMABLE,
  GRPC_BUFFER_USER_PENDING_RECLAIM,
  GRPC_BUFFER_USER_COUNT
} grpc_buffer_user_list;

struct grpc_buffer_user {
  grpc_buffer_pool *buffer_pool;
  struct {
    gpr_slice_buffer *target;
    size_t allocate_slices;
    size_t allocate_slice_size;
    grpc_closure *on_done;
  } pending_allocation;
  grpc_workqueue *response_workqueue;

  struct {
    grpc_buffer_user *next;
    grpc_buffer_user *prev;
  } links[GRPC_BUFFER_USER_COUNT];

  grpc_closure queue_alloc;

  struct {
    bool has_outstanding_request;
  } reclaimation_state;
};

void grpc_buffer_user_init(grpc_buffer_user *buffer_user,
                           grpc_buffer_pool *buffer_pool,
                           grpc_workqueue *response_workqueue);
void grpc_buffer_user_destroy(grpc_buffer_user *buffer_user,
                              grpc_closure *on_done);

void grpc_buffer_user_alloc(grpc_exec_ctx *exec_ctx,
                            grpc_buffer_user *buffer_user,
                            size_t target_slice_count, size_t target_slice_size,
                            gpr_slice_buffer *dest_slice_buffer,
                            grpc_closure *on_done);
void grpc_buffer_user_set_cleanup_function(
    grpc_exec_ctx *exec_ctx, grpc_buffer_user *buffer_user,
    grpc_buffer_user_cleanup_function cleanup_function,
    void *cleanup_function_arg);

#endif  // GRPC_CORE_LIB_IOMGR_BUFFER_POOL_H
