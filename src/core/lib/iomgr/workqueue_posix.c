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

#include <grpc/support/port_platform.h>

#ifdef GPR_POSIX_SOCKET

#include "src/core/lib/iomgr/workqueue.h"

#include <stdio.h>

#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/useful.h>

#include "src/core/lib/iomgr/ev_posix.h"

struct grpc_workqueue {
  gpr_refcount refs;

  gpr_mu mu;
  grpc_closure_list closure_list;

  grpc_wakeup_fd wakeup_fd;
  struct grpc_fd *wakeup_read_fd;

  grpc_closure read_closure;
};

static void on_readable(grpc_exec_ctx *exec_ctx, void *arg, grpc_error *error);

grpc_error *grpc_workqueue_create(grpc_exec_ctx *exec_ctx,
                                  grpc_workqueue **workqueue) {
  char name[32];
  *workqueue = gpr_malloc(sizeof(grpc_workqueue));
  (*workqueue)->closure_list.head = (*workqueue)->closure_list.tail = NULL;
  grpc_error *err = grpc_wakeup_fd_init(&(*workqueue)->wakeup_fd);
  if (err != GRPC_ERROR_NONE) {
    gpr_free(*workqueue);
    return err;
  }
  sprintf(name, "workqueue:%p", (void *)(*workqueue));
  err = grpc_fd_create(
      exec_ctx, GRPC_WAKEUP_FD_GET_READ_FD(&(*workqueue)->wakeup_fd),
      GRPC_FD_FLAG_NO_WORKQUEUE, name, &(*workqueue)->wakeup_read_fd);
  if (err != GRPC_ERROR_NONE) {
    grpc_wakeup_fd_destroy(&(*workqueue)->wakeup_fd);
    gpr_free(*workqueue);
    return err;
  }
  gpr_ref_init(&(*workqueue)->refs, 1);
  gpr_mu_init(&(*workqueue)->mu);
  grpc_closure_init(&(*workqueue)->read_closure, on_readable, *workqueue);
  grpc_fd_notify_on_read(exec_ctx, (*workqueue)->wakeup_read_fd,
                         &(*workqueue)->read_closure);
  return GRPC_ERROR_NONE;
}

static void workqueue_destroy(grpc_exec_ctx *exec_ctx,
                              grpc_workqueue *workqueue) {
  GPR_ASSERT(grpc_closure_list_empty(workqueue->closure_list));
  grpc_fd_shutdown(exec_ctx, workqueue->wakeup_read_fd);
}

#ifdef GRPC_WORKQUEUE_REFCOUNT_DEBUG
grpc_workqueue *grpc_workqueue_ref(grpc_workqueue *workqueue, const char *file,
                                   int line, const char *reason) {
  gpr_log(file, line, GPR_LOG_SEVERITY_DEBUG, "WORKQUEUE:%p   ref %d -> %d %s",
          workqueue, (int)workqueue->refs.count, (int)workqueue->refs.count + 1,
          reason);
#else
grpc_workqueue *grpc_workqueue_ref(grpc_workqueue *workqueue) {
#endif
  gpr_ref(&workqueue->refs);
  return workqueue;
}

#ifdef GRPC_WORKQUEUE_REFCOUNT_DEBUG
void grpc_workqueue_unref(grpc_exec_ctx *exec_ctx, grpc_workqueue *workqueue,
                          const char *file, int line, const char *reason) {
  gpr_log(file, line, GPR_LOG_SEVERITY_DEBUG, "WORKQUEUE:%p unref %d -> %d %s",
          workqueue, (int)workqueue->refs.count, (int)workqueue->refs.count - 1,
          reason);
#else
void grpc_workqueue_unref(grpc_exec_ctx *exec_ctx, grpc_workqueue *workqueue) {
#endif
  if (gpr_unref(&workqueue->refs)) {
    workqueue_destroy(exec_ctx, workqueue);
  }
}

void grpc_workqueue_add_to_pollset(grpc_exec_ctx *exec_ctx,
                                   grpc_workqueue *workqueue,
                                   grpc_pollset *pollset) {
  grpc_pollset_add_fd(exec_ctx, pollset, workqueue->wakeup_read_fd);
}

void grpc_workqueue_flush(grpc_exec_ctx *exec_ctx, grpc_workqueue *workqueue) {
  gpr_mu_lock(&workqueue->mu);
  grpc_exec_ctx_enqueue_list(exec_ctx, &workqueue->closure_list, NULL);
  gpr_mu_unlock(&workqueue->mu);
}

static void on_readable(grpc_exec_ctx *exec_ctx, void *arg, grpc_error *error) {
  grpc_workqueue *workqueue = arg;

  if (error != GRPC_ERROR_NONE) {
    gpr_mu_destroy(&workqueue->mu);
    /* HACK: let wakeup_fd code know that we stole the fd */
    workqueue->wakeup_fd.read_fd = 0;
    grpc_wakeup_fd_destroy(&workqueue->wakeup_fd);
    grpc_fd_orphan(exec_ctx, workqueue->wakeup_read_fd, NULL, NULL, "destroy");
    gpr_free(workqueue);
  } else {
    gpr_mu_lock(&workqueue->mu);
    grpc_exec_ctx_enqueue_list(exec_ctx, &workqueue->closure_list, NULL);
    error = grpc_wakeup_fd_consume_wakeup(&workqueue->wakeup_fd);
    gpr_mu_unlock(&workqueue->mu);
    if (error == GRPC_ERROR_NONE) {
      grpc_fd_notify_on_read(exec_ctx, workqueue->wakeup_read_fd,
                             &workqueue->read_closure);
    } else {
      /* recurse to get error handling */
      on_readable(exec_ctx, arg, error);
    }
  }
}

void grpc_workqueue_enqueue(grpc_exec_ctx *exec_ctx, grpc_workqueue *workqueue,
                            grpc_closure *closure, grpc_error *error) {
  grpc_error *push_error = GRPC_ERROR_NONE;
  gpr_mu_lock(&workqueue->mu);
  if (grpc_closure_list_empty(workqueue->closure_list)) {
    push_error = grpc_wakeup_fd_wakeup(&workqueue->wakeup_fd);
  }
  grpc_closure_list_append(&workqueue->closure_list, closure, error);
  if (push_error != GRPC_ERROR_NONE) {
    const char *msg = grpc_error_string(push_error);
    gpr_log(GPR_ERROR, "Failed to push to workqueue: %s", msg);
    grpc_error_free_string(msg);
    grpc_exec_ctx_enqueue_list(exec_ctx, &workqueue->closure_list, NULL);
  }
  gpr_mu_unlock(&workqueue->mu);
}

#endif /* GPR_POSIX_SOCKET */
