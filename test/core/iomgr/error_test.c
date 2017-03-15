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

#include "src/core/lib/iomgr/error.h"

#include <grpc/grpc.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/thd.h>
#include <grpc/support/useful.h>

#include <string.h>

#include "test/core/util/test_config.h"

static void test_set_get_int() {
  grpc_error* error = GRPC_ERROR_CREATE("Test");
  GPR_ASSERT(error);
  intptr_t i = 0;
  GPR_ASSERT(grpc_error_get_int(error, GRPC_ERROR_INT_FILE_LINE, &i));
  GPR_ASSERT(i);  // line set will never be 0
  GPR_ASSERT(!grpc_error_get_int(error, GRPC_ERROR_INT_ERRNO, &i));
  GPR_ASSERT(!grpc_error_get_int(error, GRPC_ERROR_INT_SIZE, &i));

  intptr_t errnumber = 314;
  error = grpc_error_set_int(error, GRPC_ERROR_INT_ERRNO, errnumber);
  GPR_ASSERT(grpc_error_get_int(error, GRPC_ERROR_INT_ERRNO, &i));
  GPR_ASSERT(i == errnumber);

  intptr_t http = 2;
  error = grpc_error_set_int(error, GRPC_ERROR_INT_HTTP2_ERROR, http);
  GPR_ASSERT(grpc_error_get_int(error, GRPC_ERROR_INT_HTTP2_ERROR, &i));
  GPR_ASSERT(i == http);

  GRPC_ERROR_UNREF(error);
}

static void test_set_get_str() {
  grpc_error* error = GRPC_ERROR_CREATE("Test");

  GPR_ASSERT(!grpc_error_get_str(error, GRPC_ERROR_STR_SYSCALL));
  GPR_ASSERT(!grpc_error_get_str(error, GRPC_ERROR_STR_TSI_ERROR));

  const char* c = grpc_error_get_str(error, GRPC_ERROR_STR_FILE);
  GPR_ASSERT(c);
  GPR_ASSERT(strstr(c, "error_test.c"));  // __FILE__ expands differently on
                                          // Windows. All should at least
                                          // contain error_test.c

  c = grpc_error_get_str(error, GRPC_ERROR_STR_DESCRIPTION);
  GPR_ASSERT(c);
  GPR_ASSERT(!strcmp(c, "Test"));

  error =
      grpc_error_set_str(error, GRPC_ERROR_STR_GRPC_MESSAGE, "longer message");
  c = grpc_error_get_str(error, GRPC_ERROR_STR_GRPC_MESSAGE);
  GPR_ASSERT(c);
  GPR_ASSERT(!strcmp(c, "longer message"));

  GRPC_ERROR_UNREF(error);
}

static void test_copy_and_unref() {
  // error1 has one ref
  grpc_error* error1 = grpc_error_set_str(
      GRPC_ERROR_CREATE("Test"), GRPC_ERROR_STR_GRPC_MESSAGE, "message");
  const char* c = grpc_error_get_str(error1, GRPC_ERROR_STR_GRPC_MESSAGE);
  GPR_ASSERT(c);
  GPR_ASSERT(!strcmp(c, "message"));

  // error 1 has two refs
  GRPC_ERROR_REF(error1);
  // this gives error3 a ref to the new error, and decrements error1 to one ref
  grpc_error* error3 =
      grpc_error_set_str(error1, GRPC_ERROR_STR_SYSCALL, "syscall");
  GPR_ASSERT(error3 != error1);  // should not be the same because of extra ref
  c = grpc_error_get_str(error3, GRPC_ERROR_STR_GRPC_MESSAGE);
  GPR_ASSERT(c);
  GPR_ASSERT(!strcmp(c, "message"));

  // error 1 should not have a syscall but 3 should
  GPR_ASSERT(!grpc_error_get_str(error1, GRPC_ERROR_STR_SYSCALL));
  c = grpc_error_get_str(error3, GRPC_ERROR_STR_SYSCALL);
  GPR_ASSERT(c);
  GPR_ASSERT(!strcmp(c, "syscall"));

  GRPC_ERROR_UNREF(error1);
  GRPC_ERROR_UNREF(error3);
}

static void test_create_referencing() {
  grpc_error* child = grpc_error_set_str(
      GRPC_ERROR_CREATE("Child"), GRPC_ERROR_STR_GRPC_MESSAGE, "message");
  grpc_error* parent = GRPC_ERROR_CREATE_REFERENCING("Parent", &child, 1);
  GPR_ASSERT(parent);

  GRPC_ERROR_UNREF(child);
  GRPC_ERROR_UNREF(parent);
}

static void test_create_referencing_many() {
  grpc_error* children[3];
  children[0] = grpc_error_set_str(GRPC_ERROR_CREATE("Child1"),
                                   GRPC_ERROR_STR_GRPC_MESSAGE, "message");
  children[1] = grpc_error_set_int(GRPC_ERROR_CREATE("Child2"),
                                   GRPC_ERROR_INT_HTTP2_ERROR, 5);
  children[2] = grpc_error_set_str(GRPC_ERROR_CREATE("Child3"),
                                   GRPC_ERROR_STR_GRPC_MESSAGE, "message 3");

  grpc_error* parent = GRPC_ERROR_CREATE_REFERENCING("Parent", children, 3);
  GPR_ASSERT(parent);

  for (size_t i = 0; i < 3; ++i) {
    GRPC_ERROR_UNREF(children[i]);
  }
  GRPC_ERROR_UNREF(parent);
}

static void print_error_string() {
  grpc_error* error =
      grpc_error_set_int(GRPC_ERROR_CREATE("Error"), GRPC_ERROR_INT_GRPC_STATUS,
                         GRPC_STATUS_UNIMPLEMENTED);
  error = grpc_error_set_int(error, GRPC_ERROR_INT_SIZE, 666);
  error = grpc_error_set_str(error, GRPC_ERROR_STR_GRPC_MESSAGE, "message");
  // gpr_log(GPR_DEBUG, "%s", grpc_error_string(error));
  GRPC_ERROR_UNREF(error);
}

static void print_error_string_reference() {
  grpc_error* children[2];
  children[0] = grpc_error_set_str(
      grpc_error_set_int(GRPC_ERROR_CREATE("1"), GRPC_ERROR_INT_GRPC_STATUS,
                         GRPC_STATUS_UNIMPLEMENTED),
      GRPC_ERROR_STR_GRPC_MESSAGE, "message for child 1");
  children[1] = grpc_error_set_str(
      grpc_error_set_int(GRPC_ERROR_CREATE("2sd"), GRPC_ERROR_INT_GRPC_STATUS,
                         GRPC_STATUS_INTERNAL),
      GRPC_ERROR_STR_GRPC_MESSAGE, "message for child 2");

  grpc_error* parent = GRPC_ERROR_CREATE_REFERENCING("Parent", children, 2);

  gpr_log(GPR_DEBUG, "%s", grpc_error_string(parent));

  for (size_t i = 0; i < 2; ++i) {
    GRPC_ERROR_UNREF(children[i]);
  }
  GRPC_ERROR_UNREF(parent);
}

static void test_os_error() {
  int fake_errno = 5;
  const char* syscall = "syscall name";
  grpc_error* error = GRPC_OS_ERROR(fake_errno, syscall);

  intptr_t i = 0;
  GPR_ASSERT(grpc_error_get_int(error, GRPC_ERROR_INT_ERRNO, &i));
  GPR_ASSERT(i == fake_errno);

  const char* c = grpc_error_get_str(error, GRPC_ERROR_STR_SYSCALL);
  GPR_ASSERT(c);
  GPR_ASSERT(!strcmp(c, syscall));
  GRPC_ERROR_UNREF(error);
}

static void test_special() {
  grpc_error* error = GRPC_ERROR_NONE;
  error = grpc_error_add_child(error, GRPC_ERROR_CREATE("test child"));
  intptr_t i;
  GPR_ASSERT(grpc_error_get_int(error, GRPC_ERROR_INT_GRPC_STATUS, &i));
  GPR_ASSERT(i == GRPC_STATUS_OK);
  GRPC_ERROR_UNREF(error);
}

int main(int argc, char** argv) {
  grpc_test_init(argc, argv);
  grpc_init();
  test_set_get_int();
  test_set_get_str();
  test_copy_and_unref();
  print_error_string();
  print_error_string_reference();
  test_os_error();
  test_create_referencing();
  test_create_referencing_many();
  test_special();
  grpc_shutdown();

  return 0;
}
