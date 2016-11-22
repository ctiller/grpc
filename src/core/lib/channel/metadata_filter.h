/*
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

#ifndef C_METADATA_FILTER_H
#define C_METADATA_FILTER_H

#include "src/core/lib/channel/channel_stack_fwd.h"
#include "src/core/lib/iomgr/error.h"
#include "src/core/lib/transport/metadata.h"
#include "src/core/lib/transport/metadata_batch.h"

typedef struct grpc_metadata_filter grpc_metadata_filter;

/* Filter expression */

#define GRPC_METADATA_FILTER_KEY_WILDCARD ":*:"
#define GRPC_METADATA_FILTER_KEY_START_OF_LIST ":start:"
#define GRPC_METADATA_FILTER_KEY_END_OF_LIST ":end:"

typedef struct {
  grpc_error *error;
  grpc_mdelem elem;
} grpc_filtered_metadata;

#define GRPC_FILTERED_METADATA_OK(elem) \
  ((grpc_filtered_metadata){GRPC_ERROR_NONE, (elem)})
#define GRPC_FILTERED_METADATA_ERROR(error) \
  ((grpc_filtered_metadata){(error), GRPC_MDNULL})

typedef grpc_filtered_metadata (*grpc_metadata_filter_func)(
    grpc_exec_ctx *exec_ctx, grpc_call_element *call_elem,
    grpc_mdelem metadata);

/* Filter building */

typedef struct grpc_metadata_filter_builder grpc_metadata_filter_builder;

grpc_metadata_filter_builder *grpc_metadata_filter_builder_create(void);
void grpc_metadata_filter_builder_destroy(
    grpc_metadata_filter_builder *builder);

void grpc_metadata_filter_builder_add_filter(
    grpc_metadata_filter_builder *builder, const char *key, size_t level,
    grpc_metadata_filter_func func);

size_t grpc_metadata_filter_builder_required_size(
    grpc_metadata_filter_builder *builder);

grpc_metadata_filter *grpc_metadata_filter_builder_finish(
    grpc_metadata_filter_builder *builder, void *memory);

/* Runtime */

void grpc_metadata_filter_destroy(grpc_metadata_filter *filter);

void grpc_metadata_filter_filter(grpc_metadata_filter *filter,
                                 grpc_metadata_batch *batch);

#endif
