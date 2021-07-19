/*
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
 */

/*
 * Automatically generated by tools/codegen/core/gen_settings_ids.py
 */

#include <grpc/support/port_platform.h>

#include "src/core/ext/transport/chttp2/transport/http2_settings.h"

#include "src/core/lib/gpr/useful.h"
#include "src/core/lib/transport/http2_errors.h"

const uint16_t grpc_setting_id_to_wire_id[] = {1, 2, 3, 4, 5, 6, 65027};

bool grpc_wire_id_to_setting_id(uint32_t wire_id, grpc_chttp2_setting_id* out) {
  uint32_t i = wire_id - 1;
  uint32_t x = i % 256;
  uint32_t y = i / 256;
  uint32_t h = x;
  switch (y) {
    case 254:
      h += 4;
      break;
  }
  *out = static_cast<grpc_chttp2_setting_id>(h);
  return h < GPR_ARRAY_SIZE(grpc_setting_id_to_wire_id) && grpc_setting_id_to_wire_id[h] == wire_id;
}

const grpc_chttp2_setting_parameters grpc_chttp2_settings_parameters[GRPC_CHTTP2_NUM_SETTINGS] = {
    {"HEADER_TABLE_SIZE", 4096u, 0u, 4294967295u, GRPC_CHTTP2_CLAMP_INVALID_VALUE,
     GRPC_HTTP2_PROTOCOL_ERROR},
    {"ENABLE_PUSH", 1u, 0u, 1u, GRPC_CHTTP2_DISCONNECT_ON_INVALID_VALUE, GRPC_HTTP2_PROTOCOL_ERROR},
    {"MAX_CONCURRENT_STREAMS", 4294967295u, 0u, 4294967295u,
     GRPC_CHTTP2_DISCONNECT_ON_INVALID_VALUE, GRPC_HTTP2_PROTOCOL_ERROR},
    {"INITIAL_WINDOW_SIZE", 65535u, 0u, 2147483647u, GRPC_CHTTP2_DISCONNECT_ON_INVALID_VALUE,
     GRPC_HTTP2_FLOW_CONTROL_ERROR},
    {"MAX_FRAME_SIZE", 16384u, 16384u, 16777215u, GRPC_CHTTP2_DISCONNECT_ON_INVALID_VALUE,
     GRPC_HTTP2_PROTOCOL_ERROR},
    {"MAX_HEADER_LIST_SIZE", 16777216u, 0u, 16777216u, GRPC_CHTTP2_CLAMP_INVALID_VALUE,
     GRPC_HTTP2_PROTOCOL_ERROR},
    {"GRPC_ALLOW_TRUE_BINARY_METADATA", 0u, 0u, 1u, GRPC_CHTTP2_CLAMP_INVALID_VALUE,
     GRPC_HTTP2_PROTOCOL_ERROR},
};
