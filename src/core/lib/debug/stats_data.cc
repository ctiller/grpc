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
 * Automatically generated by tools/codegen/core/gen_stats_data.py
 */

#include <grpc/support/port_platform.h>

#include "src/core/lib/debug/stats_data.h"

#include <stdint.h>

#include "src/core/lib/debug/stats.h"

namespace {
union DblUint {
  double dbl;
  uint64_t uint;
};
}  // namespace
const char* grpc_stats_counter_name[GRPC_STATS_COUNTER_COUNT] = {
    "client_calls_created",
    "server_calls_created",
    "client_channels_created",
    "client_subchannels_created",
    "server_channels_created",
    "syscall_write",
    "syscall_read",
    "tcp_read_alloc_8k",
    "tcp_read_alloc_64k",
    "http2_settings_writes",
    "http2_pings_sent",
    "http2_writes_begun",
    "http2_transport_stalls",
    "http2_stream_stalls",
    "cq_pluck_creates",
    "cq_next_creates",
    "cq_callback_creates",
};
const char* grpc_stats_counter_doc[GRPC_STATS_COUNTER_COUNT] = {
    "Number of client side calls created by this process",
    "Number of server side calls created by this process",
    "Number of client channels created",
    "Number of client subchannels created",
    "Number of server channels created",
    "Number of write syscalls (or equivalent - eg sendmsg) made by this "
    "process",
    "Number of read syscalls (or equivalent - eg recvmsg) made by this process",
    "Number of 8k allocations by the TCP subsystem for reading",
    "Number of 64k allocations by the TCP subsystem for reading",
    "Number of settings frames sent",
    "Number of HTTP2 pings sent by process",
    "Number of HTTP2 writes initiated",
    "Number of times sending was completely stalled by the transport flow "
    "control window",
    "Number of times sending was completely stalled by the stream flow control "
    "window",
    "Number of completion queues created for cq_pluck (indicates sync api "
    "usage)",
    "Number of completion queues created for cq_next (indicates cq async api "
    "usage)",
    "Number of completion queues created for cq_callback (indicates callback "
    "api usage)",
};
const char* grpc_stats_histogram_name[GRPC_STATS_HISTOGRAM_COUNT] = {
    "call_initial_size",       "tcp_write_size", "tcp_write_iov_size",
    "tcp_read_size",           "tcp_read_offer", "tcp_read_offer_iov_size",
    "http2_send_message_size",
};
const char* grpc_stats_histogram_doc[GRPC_STATS_HISTOGRAM_COUNT] = {
    "Initial size of the grpc_call arena created at call start",
    "Number of bytes offered to each syscall_write",
    "Number of byte segments offered to each syscall_write",
    "Number of bytes received by each syscall_read",
    "Number of bytes offered to each syscall_read",
    "Number of byte segments offered to each syscall_read",
    "Size of messages received by HTTP2 transport",
};
const int grpc_stats_table_0[65] = {
    0,       1,       2,       3,       4,       6,       8,        11,
    15,      20,      26,      34,      44,      57,      73,       94,
    121,     155,     199,     255,     327,     419,     537,      688,
    881,     1128,    1444,    1848,    2365,    3026,    3872,     4954,
    6338,    8108,    10373,   13270,   16976,   21717,   27782,    35541,
    45467,   58165,   74409,   95189,   121772,  155778,  199281,   254933,
    326126,  417200,  533707,  682750,  873414,  1117323, 1429345,  1828502,
    2339127, 2992348, 3827987, 4896985, 6264509, 8013925, 10251880, 13114801,
    16777216};
const uint8_t grpc_stats_table_1[87] = {
    0,  0,  1,  1,  2,  3,  3,  4,  4,  5,  6,  6,  7,  8,  8,  9,  10, 11,
    11, 12, 13, 13, 14, 15, 15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 22, 23,
    24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36,
    36, 37, 38, 39, 39, 40, 41, 41, 42, 43, 44, 44, 45, 45, 46, 47, 48, 48,
    49, 50, 51, 51, 52, 53, 53, 54, 55, 56, 56, 57, 58, 58, 59};
const int grpc_stats_table_2[65] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,
    14,  16,  18,  20,  22,  24,  27,  30,  33,  36,  39,  43,  47,
    51,  56,  61,  66,  72,  78,  85,  92,  100, 109, 118, 128, 139,
    151, 164, 178, 193, 209, 226, 244, 264, 285, 308, 333, 359, 387,
    418, 451, 486, 524, 565, 609, 656, 707, 762, 821, 884, 952, 1024};
const uint8_t grpc_stats_table_3[102] = {
    0,  0,  0,  1,  1,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,
    6,  7,  7,  7,  8,  8,  9,  9,  10, 11, 11, 12, 12, 13, 13, 14, 14,
    14, 15, 15, 16, 16, 17, 17, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23,
    23, 24, 24, 24, 25, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 32,
    32, 33, 33, 34, 35, 35, 36, 37, 37, 38, 38, 39, 39, 40, 40, 41, 41,
    42, 42, 43, 44, 44, 45, 46, 46, 47, 48, 48, 49, 49, 50, 50, 51, 51};
const int grpc_stats_table_4[65] = {
    0,      1,      2,      3,      4,     5,     7,     9,     11,    14,
    17,     21,     26,     32,     39,    47,    57,    68,    82,    98,
    117,    140,    167,    199,    238,   284,   339,   404,   482,   575,
    685,    816,    972,    1158,   1380,  1644,  1959,  2334,  2780,  3312,
    3945,   4699,   5597,   6667,   7941,  9459,  11267, 13420, 15984, 19038,
    22676,  27009,  32169,  38315,  45635, 54353, 64737, 77104, 91834, 109378,
    130273, 155159, 184799, 220100, 262144};
const uint8_t grpc_stats_table_5[124] = {
    0,  0,  0,  1,  1,  1,  2,  2,  3,  3,  3,  4,  4,  5,  5,  6,  6,  6,
    7,  7,  7,  8,  9,  9,  10, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15,
    15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 22, 23, 24,
    24, 25, 25, 26, 26, 26, 27, 27, 28, 29, 29, 30, 30, 30, 31, 31, 32, 33,
    33, 34, 34, 34, 35, 35, 36, 37, 37, 37, 38, 38, 39, 39, 40, 40, 41, 41,
    42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 49, 50, 50,
    51, 51, 52, 52, 53, 53, 54, 54, 55, 55, 56, 56, 57, 57, 58, 58};
namespace grpc_core {
int BucketForHistogramValue_16777216_64(int value) {
  if (value < 5) {
    if (value < 0) {
      return 0;
    } else {
      return value;
    }
  } else {
    if (value < 14680065) {
      // first_nontrivial_code=4617315517961601024
      // last_code=4714142909950066688 [14680064.000000]
      DblUint val;
      val.dbl = value;
      const int bucket =
          grpc_stats_table_1[((val.uint - 4617315517961601024ull) >> 50)] + 5;
      return bucket - (value < grpc_stats_table_0[bucket]);
    } else {
      return 63;
    }
  }
}
int BucketForHistogramValue_1024_64(int value) {
  if (value < 13) {
    if (value < 0) {
      return 0;
    } else {
      return value;
    }
  } else {
    if (value < 993) {
      // first_nontrivial_code=4623507967449235456
      // last_code=4651936940097011712 [992.000000]
      DblUint val;
      val.dbl = value;
      const int bucket =
          grpc_stats_table_3[((val.uint - 4623507967449235456ull) >> 48)] + 13;
      return bucket - (value < grpc_stats_table_2[bucket]);
    } else {
      return 63;
    }
  }
}
int BucketForHistogramValue_262144_64(int value) {
  if (value < 6) {
    if (value < 0) {
      return 0;
    } else {
      return value;
    }
  } else {
    if (value < 245761) {
      // first_nontrivial_code=4618441417868443648
      // last_code=4687684262139265024 [245760.000000]
      DblUint val;
      val.dbl = value;
      const int bucket =
          grpc_stats_table_5[((val.uint - 4618441417868443648ull) >> 49)] + 6;
      return bucket - (value < grpc_stats_table_4[bucket]);
    } else {
      return 63;
    }
  }
}
}  // namespace grpc_core
const int grpc_stats_histo_buckets[7] = {64, 64, 64, 64, 64, 64, 64};
const int grpc_stats_histo_start[7] = {0, 64, 128, 192, 256, 320, 384};
const int* const grpc_stats_histo_bucket_boundaries[7] = {
    grpc_stats_table_4, grpc_stats_table_0, grpc_stats_table_2,
    grpc_stats_table_0, grpc_stats_table_0, grpc_stats_table_2,
    grpc_stats_table_0};
int (*const grpc_stats_get_bucket[7])(int value) = {
    grpc_core::BucketForHistogramValue_262144_64,
    grpc_core::BucketForHistogramValue_16777216_64,
    grpc_core::BucketForHistogramValue_1024_64,
    grpc_core::BucketForHistogramValue_16777216_64,
    grpc_core::BucketForHistogramValue_16777216_64,
    grpc_core::BucketForHistogramValue_1024_64,
    grpc_core::BucketForHistogramValue_16777216_64};
