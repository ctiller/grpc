// Copyright 2021 gRPC authors.
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

#include <grpc/support/port_platform.h>

#include "src/core/lib/gprpp/time.h"

#include <limits>

#include <grpc/impl/codegen/gpr_types.h>

namespace grpc_core {

namespace {

gpr_timespec MillisecondsAsTimespec(int64_t millis, gpr_clock_type clock_type) {
  // special-case infinities as Timestamp can be 32bit on some
  // platforms while gpr_time_from_millis always takes an int64_t.
  if (millis == std::numeric_limits<int64_t>::max()) {
    return gpr_inf_future(clock_type);
  }
  if (millis == std::numeric_limits<int64_t>::min()) {
    return gpr_inf_past(clock_type);
  }

  if (clock_type == GPR_TIMESPAN) {
    return gpr_time_from_millis(millis, GPR_TIMESPAN);
  }
  return gpr_time_add(gpr_convert_clock_type(GetGrpcStartTime(), clock_type),
                      gpr_time_from_millis(millis, GPR_TIMESPAN));
}

}  // namespace

gpr_timespec Timestamp::as_timespec(gpr_clock_type clock_type) const {
  return MillisecondsAsTimespec(millis_, clock_type);
}

gpr_timespec Duration::as_timespec() const {
  return MillisecondsAsTimespec(millis_, GPR_TIMESPAN);
}

}  // namespace grpc_core
