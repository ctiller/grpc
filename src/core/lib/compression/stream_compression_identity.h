/*
 *
 * Copyright 2017 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef GRPC_CORE_LIB_COMPRESSION_STREAM_COMPRESSION_IDENTITY_H
#define GRPC_CORE_LIB_COMPRESSION_STREAM_COMPRESSION_IDENTITY_H

#include <grpc/support/port_platform.h>

#include "src/core/lib/compression/stream_compression.h"

extern const grpc_stream_compression_vtable
    grpc_stream_compression_identity_vtable;

#endif  // GRPC_CORE_LIB_COMPRESSION_STREAM_COMPRESSION_IDENTITY_H