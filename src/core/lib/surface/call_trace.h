// Copyright 2022 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GRPC_CORE_LIB_SURFACE_CALL_TRACE_H
#define GRPC_CORE_LIB_SURFACE_CALL_TRACE_H

#include <grpc/support/port_platform.h>

#include "src/core/lib/channel/channel_fwd.h"
#include "src/core/lib/debug/trace.h"

extern grpc_core::TraceFlag grpc_call_trace;

namespace grpc_core {
const grpc_channel_filter* PromiseTracingFilterFor(
    const grpc_channel_filter* filter);
}

#endif  // GRPC_CORE_LIB_SURFACE_CALL_TRACE_H
