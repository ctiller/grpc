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

#include "src/core/lib/iomgr/closure.h"

typedef struct grpc_buffer_pool grpc_buffer_pool;

grpc_buffer_pool *grpc_buffer_pool_create(void);
void grpc_buffer_pool_resize(size_t size);
grpc_buffer_pool *grpc_buffer_pool_ref(grpc_buffer_pool *bp);
void grpc_buffer_pool_unref(grpc_buffer_pool *bp);

typedef struct grpc_buffer_user grpc_buffer_user;

typedef struct {
  void (*update_memory_usage_estimate)(grpc_buffer_user *buffer_user,
                                       double memory_usage_estimate,
                                       grpc_closure *on_done);
  void (*free_up_memory)(grpc_buffer_user *buffer_user, grpc_closure *on_done);
} grpc_buffer_user_vtable;

struct grpc_buffer_user {
  grpc_buffer_user_vtable *vtable;
  grpc_buffer_pool *buffer_pool;
};

void grpc_buffer_user_init(grpc_buffer_user *buffer_user,
                           grpc_buffer_pool *buffer_pool,
                           grpc_buffer_user_vtable *vtable);
void grpc_buffer_user_destroy(grpc_buffer_user *buffer_user);

void grpc_buffer_user_set_active(grpc_buffer_user *buffer_user, bool active);
void grpc_buffer_user_alloc(grpc_buffer_user *buffer_user, size_t size,
                            grpc_closure *on_done);
void grpc_buffer_user_free(grpc_buffer_user *buffer_user, size_t size);

#endif  // GRPC_CORE_LIB_IOMGR_BUFFER_POOL_H
