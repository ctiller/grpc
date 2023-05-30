// Copyright 2023 gRPC authors.
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

// This file is autogenerated: see
// tools/codegen/core/gen_huffman_decompressor.cc

#ifndef GRPC_TEST_CPP_MICROBENCHMARKS_HUFFMAN_GEOMETRIES_DECODE_HUFF_9_15_6_H
#define GRPC_TEST_CPP_MICROBENCHMARKS_HUFFMAN_GEOMETRIES_DECODE_HUFF_9_15_6_H
#include <grpc/support/port_platform.h>

#include <cstddef>
#include <cstdint>
namespace grpc_core {
namespace geometry_9_15_6 {
class HuffDecoderCommon {
 protected:
  static inline uint64_t GetOp2(size_t i) { return table2_0_ops_[i]; }
  static inline uint64_t GetEmit2(size_t, size_t emit) {
    return table2_0_emit_[emit];
  }
  static inline uint64_t GetOp3(size_t i) { return table3_0_ops_[i]; }
  static inline uint64_t GetEmit3(size_t, size_t emit) {
    return table3_0_emit_[emit];
  }
  static inline uint64_t GetOp4(size_t i) {
    return table4_ops_[i >> 6][i & 0x3f];
  }
  static inline uint64_t GetEmit4(size_t i, size_t emit) {
    return table4_emit_[i >> 6][emit];
  }
  static inline uint64_t GetOp5(size_t i) {
    return table5_ops_[i >> 7][i & 0x7f];
  }
  static inline uint64_t GetEmit5(size_t i, size_t emit) {
    return table5_emit_[i >> 7][emit];
  }
  static inline uint64_t GetOp1(size_t i) {
    return table1_ops_[i >> 5][i & 0x1f];
  }
  static inline uint64_t GetEmit1(size_t i, size_t emit) {
    return table1_emit_[i >> 5][emit];
  }
  static inline uint64_t GetOp6(size_t i) { return i ? 3 : 1; }
  static inline uint64_t GetEmit6(size_t, size_t emit) { return emit + 33; }
  static inline uint64_t GetOp7(size_t i) { return i ? 3 : 1; }
  static inline uint64_t GetEmit7(size_t, size_t emit) { return emit + 40; }
  static inline uint64_t GetOp9(size_t i) { return i ? 1 : 2; }
  static inline uint64_t GetEmit9(size_t, size_t emit) {
    return ((void)emit, 63);
  }
  static inline uint64_t GetOp8(size_t i) {
    return ((i < 1 ? (((void)i, 0)) : ((i - 1))) < 1
                ? (((void)(i < 1 ? (((void)i, 0)) : ((i - 1))), 1))
                : (((i < 1 ? (((void)i, 0)) : ((i - 1))) - 1) ? 10 : 6));
  }
  static inline uint64_t GetEmit8(size_t, size_t emit) {
    return (emit < 1 ? (((void)emit, 63)) : ((emit - 1) ? 43 : 39));
  }
  static inline uint64_t GetOp11(size_t i) {
    return ((i < 2 ? (i) : ((i - 2) + 1)) < 1
                ? (((void)(i < 2 ? (i) : ((i - 2) + 1)), 2))
                : (((i < 2 ? (i) : ((i - 2) + 1)) - 1) ? 0 : 1));
  }
  static inline uint64_t GetEmit11(size_t, size_t emit) {
    return ((void)emit, 124);
  }
  static inline uint64_t GetOp12(size_t i) {
    return table12_0_inner_[table12_0_outer_[i]];
  }
  static inline uint64_t GetEmit12(size_t, size_t emit) {
    return (emit < 1 ? (((void)emit, 124)) : ((emit - 1) ? 62 : 35));
  }
  static inline uint64_t GetOp13(size_t i) {
    return table13_0_inner_[table13_0_outer_[i]];
  }
  static inline uint64_t GetEmit13(size_t, size_t emit) {
    return table13_0_emit_[emit];
  }
  static inline uint64_t GetOp14(size_t i) { return table14_0_ops_[i]; }
  static inline uint64_t GetEmit14(size_t, size_t emit) {
    return table14_0_emit_[emit];
  }
  static inline uint64_t GetOp15(size_t i) { return table15_0_ops_[i]; }
  static inline uint64_t GetEmit15(size_t, size_t emit) {
    return table15_0_emit_[emit];
  }
  static inline uint64_t GetOp16(size_t i) { return table16_0_ops_[i]; }
  static inline uint64_t GetEmit16(size_t, size_t emit) {
    return table15_0_emit_[emit];
  }
  static inline uint64_t GetOp17(size_t i) { return table17_0_ops_[i]; }
  static inline uint64_t GetEmit17(size_t, size_t emit) {
    return table15_0_emit_[emit];
  }
  static inline uint64_t GetOp18(size_t i) {
    return table18_ops_[i >> 6][i & 0x3f];
  }
  static inline uint64_t GetEmit18(size_t i, size_t emit) {
    return table18_emit_[i >> 6][emit];
  }
  static inline uint64_t GetOp19(size_t i) {
    return table19_ops_[i >> 6][i & 0x3f];
  }
  static inline uint64_t GetEmit19(size_t i, size_t emit) {
    return table19_emit_[i >> 6][emit];
  }
  static inline uint64_t GetOp20(size_t i) {
    return table20_ops_[i >> 7][i & 0x7f];
  }
  static inline uint64_t GetEmit20(size_t i, size_t emit) {
    return table20_emit_[i >> 7][emit];
  }
  static inline uint64_t GetOp21(size_t i) {
    return table21_ops_[i >> 7][i & 0x7f];
  }
  static inline uint64_t GetEmit21(size_t i, size_t emit) {
    return table21_emit_[i >> 7][emit];
  }
  static inline uint64_t GetOp22(size_t i) {
    return table22_ops_[i >> 8][i & 0xff];
  }
  static inline uint64_t GetEmit22(size_t i, size_t emit) {
    return table22_emit_[i >> 8][emit];
  }
  static inline uint64_t GetOp23(size_t i) {
    return table23_ops_[i >> 8][i & 0xff];
  }
  static inline uint64_t GetEmit23(size_t i, size_t emit) {
    return table23_emit_[i >> 8][emit];
  }
  static inline uint64_t GetOp10(size_t i) {
    return table10_ops_[i >> 9][i & 0x1ff];
  }
  static inline uint64_t GetEmit10(size_t i, size_t emit) {
    return table10_emit_[i >> 9][emit];
  }
  static inline uint64_t GetOp24(size_t i) { return i ? 3 : 1; }
  static inline uint64_t GetEmit24(size_t, size_t emit) {
    return emit ? 207 : 199;
  }
  static inline uint64_t GetOp25(size_t i) { return i ? 3 : 1; }
  static inline uint64_t GetEmit25(size_t, size_t emit) { return emit + 234; }
  static inline uint64_t GetOp26(size_t i) {
    return (i < 2 ? (i ? 6 : 2) : ((i - 2) ? 14 : 10));
  }
  static inline uint64_t GetEmit26(size_t, size_t emit) {
    return (emit < 2 ? (emit + 192) : ((emit - 2) + 200));
  }
  static inline uint64_t GetOp27(size_t i) {
    return (i < 2 ? (i ? 6 : 2) : ((i - 2) ? 14 : 10));
  }
  static inline uint64_t GetEmit27(size_t, size_t emit) {
    return (emit < 2 ? (emit ? 205 : 202) : ((emit - 2) ? 213 : 210));
  }
  static inline uint64_t GetOp28(size_t i) {
    return (i < 2 ? (i ? 6 : 2) : ((i - 2) ? 14 : 10));
  }
  static inline uint64_t GetEmit28(size_t, size_t emit) {
    return (emit < 2 ? (emit + 218) : ((emit - 2) ? 240 : 238));
  }
  static inline uint64_t GetOp29(size_t i) { return table29_0_inner_[i]; }
  static inline uint64_t GetEmit29(size_t, size_t emit) {
    return table29_0_emit_[emit];
  }
  static inline uint64_t GetOp30(size_t i) { return table29_0_inner_[i]; }
  static inline uint64_t GetEmit30(size_t, size_t emit) {
    return (emit < 4 ? (emit + 245) : ((emit - 4) + 250));
  }
  static inline uint64_t GetOp32(size_t i) {
    return (i < 1 ? (((void)i, 0)) : (((void)(i - 1), 1))) ? 1 : 2;
  }
  static inline uint64_t GetEmit32(size_t, size_t emit) {
    return ((void)emit, 254);
  }
  static inline uint64_t GetOp31(size_t i) {
    return table31_0_inner_[(i < 1 ? (((void)i, 0)) : ((i - 1)))];
  }
  static inline uint64_t GetEmit31(size_t, size_t emit) {
    return table31_0_emit_[emit];
  }
  static inline uint64_t GetOp34(size_t i) {
    return (i < 2 ? (i ? 6 : 2) : ((i - 2) ? 1 : 10));
  }
  static inline uint64_t GetEmit34(size_t, size_t emit) {
    return (emit < 1 ? (((void)emit, 242)) : ((emit - 1) ? 255 : 243));
  }
  static inline uint64_t GetOp33(size_t i) {
    return table33_0_inner_[(i < 5 ? (i / 2 + 0) : ((i - 5) + 2))];
  }
  static inline uint64_t GetEmit33(size_t, size_t emit) {
    return table33_0_emit_[emit];
  }
  static inline uint64_t GetOp36(size_t i) { return table36_0_inner_[i]; }
  static inline uint64_t GetEmit36(size_t, size_t emit) {
    return table36_0_emit_[emit];
  }
  static inline uint64_t GetOp37(size_t i) { return table37_0_ops_[i]; }
  static inline uint64_t GetEmit37(size_t, size_t emit) {
    return table36_0_emit_[emit];
  }
  static inline uint64_t GetOp35(size_t i) {
    return table35_ops_[i >> 5][i & 0x1f];
  }
  static inline uint64_t GetEmit35(size_t i, size_t emit) {
    return table35_emit_[i >> 5][emit];
  }

 private:
  static const uint8_t table2_0_emit_[10];
  static const uint8_t table2_0_ops_[32];
  static const uint8_t table3_0_emit_[36];
  static const uint8_t table3_0_ops_[64];
  static const uint8_t table4_0_emit_[22];
  static const uint8_t table4_0_ops_[64];
  static const uint8_t table4_1_emit_[46];
  static const uint8_t table4_1_ops_[64];
  static const uint8_t* const table4_emit_[2];
  static const uint8_t* const table4_ops_[2];
  static const uint8_t table5_0_ops_[128];
  static const uint8_t table5_1_emit_[52];
  static const uint8_t table5_1_ops_[128];
  static const uint8_t* const table5_emit_[2];
  static const uint8_t* const table5_ops_[2];
  static const uint8_t table1_0_emit_[2];
  static const uint16_t table1_0_ops_[32];
  static const uint8_t table1_1_emit_[2];
  static const uint8_t table1_2_emit_[2];
  static const uint8_t table1_3_emit_[2];
  static const uint8_t table1_4_emit_[2];
  static const uint8_t table1_5_emit_[4];
  static const uint16_t table1_5_ops_[32];
  static const uint8_t table1_6_emit_[4];
  static const uint8_t table1_7_emit_[4];
  static const uint8_t table1_8_emit_[4];
  static const uint8_t table1_9_emit_[4];
  static const uint8_t table1_10_emit_[4];
  static const uint8_t table1_11_emit_[6];
  static const uint16_t table1_11_ops_[32];
  static const uint8_t table1_12_emit_[8];
  static const uint16_t table1_12_ops_[32];
  static const uint8_t table1_13_emit_[8];
  static const uint8_t table1_14_emit_[8];
  static const uint8_t table1_15_emit_[10];
  static const uint16_t table1_15_ops_[32];
  static const uint8_t* const table1_emit_[16];
  static const uint16_t* const table1_ops_[16];
  static const uint8_t table12_0_inner_[5];
  static const uint8_t table12_0_outer_[8];
  static const uint8_t table13_0_emit_[9];
  static const uint8_t table13_0_inner_[11];
  static const uint8_t table13_0_outer_[16];
  static const uint8_t table14_0_emit_[11];
  static const uint8_t table14_0_ops_[32];
  static const uint8_t table15_0_emit_[14];
  static const uint8_t table15_0_ops_[64];
  static const uint8_t table16_0_ops_[128];
  static const uint8_t table17_0_ops_[256];
  static const uint8_t table18_0_ops_[64];
  static const uint8_t table18_1_emit_[1];
  static const uint8_t table18_1_ops_[64];
  static const uint8_t table18_2_emit_[1];
  static const uint8_t table18_3_emit_[1];
  static const uint8_t table18_4_emit_[2];
  static const uint8_t table18_4_ops_[64];
  static const uint8_t table18_5_emit_[2];
  static const uint8_t table18_6_emit_[2];
  static const uint8_t table18_7_emit_[5];
  static const uint8_t table18_7_ops_[64];
  static const uint8_t* const table18_emit_[8];
  static const uint8_t* const table18_ops_[8];
  static const uint8_t table19_8_emit_[1];
  static const uint8_t table19_9_emit_[1];
  static const uint8_t table19_10_emit_[1];
  static const uint8_t table19_11_emit_[1];
  static const uint8_t table19_12_emit_[1];
  static const uint8_t table19_13_emit_[1];
  static const uint8_t table19_14_emit_[2];
  static const uint8_t table19_15_emit_[6];
  static const uint8_t table19_15_ops_[64];
  static const uint8_t* const table19_emit_[16];
  static const uint8_t* const table19_ops_[16];
  static const uint8_t table20_0_ops_[128];
  static const uint8_t table20_3_ops_[128];
  static const uint8_t table20_14_ops_[128];
  static const uint8_t table20_15_emit_[14];
  static const uint8_t table20_15_ops_[128];
  static const uint8_t* const table20_emit_[16];
  static const uint8_t* const table20_ops_[16];
  static const uint8_t table21_28_emit_[1];
  static const uint8_t table21_29_emit_[1];
  static const uint8_t table21_30_emit_[2];
  static const uint8_t table21_31_emit_[25];
  static const uint8_t table21_31_ops_[128];
  static const uint8_t* const table21_emit_[32];
  static const uint8_t* const table21_ops_[32];
  static const uint8_t table22_0_ops_[256];
  static const uint8_t table22_7_ops_[256];
  static const uint8_t table22_30_ops_[256];
  static const uint8_t table22_31_emit_[51];
  static const uint8_t table22_31_ops_[256];
  static const uint8_t* const table22_emit_[32];
  static const uint8_t* const table22_ops_[32];
  static const uint16_t table23_0_ops_[256];
  static const uint16_t table23_15_ops_[256];
  static const uint8_t table23_60_emit_[1];
  static const uint8_t table23_61_emit_[1];
  static const uint8_t table23_62_emit_[1];
  static const uint8_t table23_63_emit_[79];
  static const uint16_t table23_63_ops_[256];
  static const uint8_t* const table23_emit_[64];
  static const uint16_t* const table23_ops_[64];
  static const uint16_t table10_0_ops_[512];
  static const uint16_t table10_16_ops_[512];
  static const uint16_t table10_32_ops_[512];
  static const uint16_t table10_56_ops_[512];
  static const uint16_t table10_60_ops_[512];
  static const uint8_t table10_63_emit_[91];
  static const uint16_t table10_63_ops_[512];
  static const uint8_t* const table10_emit_[64];
  static const uint16_t* const table10_ops_[64];
  static const uint8_t table29_0_emit_[8];
  static const uint8_t table29_0_inner_[8];
  static const uint8_t table31_0_emit_[15];
  static const uint8_t table31_0_inner_[15];
  static const uint8_t table33_0_emit_[5];
  static const uint8_t table33_0_inner_[5];
  static const uint8_t table36_0_emit_[15];
  static const uint8_t table36_0_inner_[16];
  static const uint8_t table37_0_ops_[32];
  static const uint8_t table35_0_emit_[8];
  static const uint8_t table35_0_ops_[32];
  static const uint8_t table35_1_emit_[10];
  static const uint8_t table35_1_ops_[32];
  static const uint8_t* const table35_emit_[2];
  static const uint8_t* const table35_ops_[2];
};
template <typename F>
class HuffDecoder : public HuffDecoderCommon {
 public:
  HuffDecoder(F sink, const uint8_t* begin, const uint8_t* end)
      : sink_(sink), begin_(begin), end_(end) {}
  bool Run() {
    while (!done_) {
      if (!RefillTo9()) {
        Done0();
        break;
      }
      const auto index = (buffer_ >> (buffer_len_ - 9)) & 0x1ff;
      const auto op = GetOp1(index);
      const int consumed = op & 15;
      buffer_len_ -= consumed;
      const auto emit_ofs = op >> 7;
      switch ((op >> 4) & 7) {
        case 0: {
          sink_(GetEmit1(index, emit_ofs + 0));
          break;
        }
        case 1: {
          DecodeStep0();
          break;
        }
        case 2: {
          DecodeStep1();
          break;
        }
        case 3: {
          DecodeStep2();
          break;
        }
        case 4: {
          DecodeStep3();
          break;
        }
      }
    }
    return ok_;
  }

 private:
  bool RefillTo9() {
    switch (buffer_len_) {
      case 0: {
        return Read2to8Bytes();
      }
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8: {
        return Read1to7Bytes();
      }
    }
    return true;
  }
  bool Read2to8Bytes() {
    switch (end_ - begin_) {
      case 0:
      case 1: {
        return false;
      }
      case 2: {
        Fill2();
        return true;
      }
      case 3: {
        Fill3();
        return true;
      }
      case 4: {
        Fill4();
        return true;
      }
      case 5: {
        Fill5();
        return true;
      }
      case 6: {
        Fill6();
        return true;
      }
      case 7: {
        Fill7();
        return true;
      }
      default: {
        Fill8();
        return true;
      }
    }
  }
  void Fill2() {
    buffer_ = (buffer_ << 16) | (static_cast<uint64_t>(begin_[0]) << 8) |
              (static_cast<uint64_t>(begin_[1]) << 0);
    begin_ += 2;
    buffer_len_ += 16;
  }
  void Fill3() {
    buffer_ = (buffer_ << 24) | (static_cast<uint64_t>(begin_[0]) << 16) |
              (static_cast<uint64_t>(begin_[1]) << 8) |
              (static_cast<uint64_t>(begin_[2]) << 0);
    begin_ += 3;
    buffer_len_ += 24;
  }
  void Fill4() {
    buffer_ = (buffer_ << 32) | (static_cast<uint64_t>(begin_[0]) << 24) |
              (static_cast<uint64_t>(begin_[1]) << 16) |
              (static_cast<uint64_t>(begin_[2]) << 8) |
              (static_cast<uint64_t>(begin_[3]) << 0);
    begin_ += 4;
    buffer_len_ += 32;
  }
  void Fill5() {
    buffer_ = (buffer_ << 40) | (static_cast<uint64_t>(begin_[0]) << 32) |
              (static_cast<uint64_t>(begin_[1]) << 24) |
              (static_cast<uint64_t>(begin_[2]) << 16) |
              (static_cast<uint64_t>(begin_[3]) << 8) |
              (static_cast<uint64_t>(begin_[4]) << 0);
    begin_ += 5;
    buffer_len_ += 40;
  }
  void Fill6() {
    buffer_ = (buffer_ << 48) | (static_cast<uint64_t>(begin_[0]) << 40) |
              (static_cast<uint64_t>(begin_[1]) << 32) |
              (static_cast<uint64_t>(begin_[2]) << 24) |
              (static_cast<uint64_t>(begin_[3]) << 16) |
              (static_cast<uint64_t>(begin_[4]) << 8) |
              (static_cast<uint64_t>(begin_[5]) << 0);
    begin_ += 6;
    buffer_len_ += 48;
  }
  void Fill7() {
    buffer_ = (buffer_ << 56) | (static_cast<uint64_t>(begin_[0]) << 48) |
              (static_cast<uint64_t>(begin_[1]) << 40) |
              (static_cast<uint64_t>(begin_[2]) << 32) |
              (static_cast<uint64_t>(begin_[3]) << 24) |
              (static_cast<uint64_t>(begin_[4]) << 16) |
              (static_cast<uint64_t>(begin_[5]) << 8) |
              (static_cast<uint64_t>(begin_[6]) << 0);
    begin_ += 7;
    buffer_len_ += 56;
  }
  void Fill8() {
    buffer_ = 0 | (static_cast<uint64_t>(begin_[0]) << 56) |
              (static_cast<uint64_t>(begin_[1]) << 48) |
              (static_cast<uint64_t>(begin_[2]) << 40) |
              (static_cast<uint64_t>(begin_[3]) << 32) |
              (static_cast<uint64_t>(begin_[4]) << 24) |
              (static_cast<uint64_t>(begin_[5]) << 16) |
              (static_cast<uint64_t>(begin_[6]) << 8) |
              (static_cast<uint64_t>(begin_[7]) << 0);
    begin_ += 8;
    buffer_len_ += 64;
  }
  bool Read1to7Bytes() {
    switch (end_ - begin_) {
      case 0: {
        return false;
      }
      case 1: {
        Fill1();
        return true;
      }
      case 2: {
        Fill2();
        return true;
      }
      case 3: {
        Fill3();
        return true;
      }
      case 4: {
        Fill4();
        return true;
      }
      case 5: {
        Fill5();
        return true;
      }
      case 6: {
        Fill6();
        return true;
      }
      default: {
        Fill7();
        return true;
      }
    }
  }
  void Fill1() {
    buffer_ = (buffer_ << 8) | (static_cast<uint64_t>(begin_[0]) << 0);
    begin_ += 1;
    buffer_len_ += 8;
  }
  void Done0() {
    done_ = true;
    switch (end_ - begin_) {
      case 1: {
        Fill1();
        break;
      }
    }
    switch (buffer_len_) {
      case 1:
      case 2:
      case 3:
      case 4: {
        ok_ = (buffer_ & ((1 << buffer_len_) - 1)) == (1 << buffer_len_) - 1;
        return;
      }
      case 5: {
        const auto index = buffer_ & 31;
        const auto op = GetOp2(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit2(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 6: {
        const auto index = buffer_ & 63;
        const auto op = GetOp3(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit3(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 7: {
        const auto index = buffer_ & 127;
        const auto op = GetOp4(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit4(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 8: {
        const auto index = buffer_ & 255;
        const auto op = GetOp5(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit5(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 0: {
        return;
      }
    }
  }
  void DecodeStep0() {
    if (!RefillTo1()) {
      Done1();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 1)) & 0x1;
    const auto op = GetOp6(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit6(index, emit_ofs + 0));
  }
  bool RefillTo1() {
    switch (buffer_len_) {
      case 0: {
        return Read1to8Bytes();
      }
    }
    return true;
  }
  bool Read1to8Bytes() {
    switch (end_ - begin_) {
      case 0: {
        return false;
      }
      case 1: {
        Fill1();
        return true;
      }
      case 2: {
        Fill2();
        return true;
      }
      case 3: {
        Fill3();
        return true;
      }
      case 4: {
        Fill4();
        return true;
      }
      case 5: {
        Fill5();
        return true;
      }
      case 6: {
        Fill6();
        return true;
      }
      case 7: {
        Fill7();
        return true;
      }
      default: {
        Fill8();
        return true;
      }
    }
  }
  void Done1() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep1() {
    if (!RefillTo1()) {
      Done2();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 1)) & 0x1;
    const auto op = GetOp7(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit7(index, emit_ofs + 0));
  }
  void Done2() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep2() {
    if (!RefillTo2()) {
      Done3();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 2)) & 0x3;
    const auto op = GetOp8(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit8(index, emit_ofs + 0));
  }
  bool RefillTo2() {
    switch (buffer_len_) {
      case 0: {
        return Read1to8Bytes();
      }
      case 1: {
        return Read1to7Bytes();
      }
    }
    return true;
  }
  void Done3() {
    done_ = true;
    switch (buffer_len_) {
      case 1: {
        const auto index = buffer_ & 1;
        const auto op = GetOp9(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit9(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 0: {
        ok_ = false;
        return;
      }
    }
  }
  void DecodeStep3() {
    if (!RefillTo15()) {
      Done4();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 15)) & 0x7fff;
    const auto op = GetOp10(index);
    const int consumed = op & 15;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 8;
    switch ((op >> 4) & 15) {
      case 0: {
        sink_(GetEmit10(index, emit_ofs + 0));
        break;
      }
      case 1: {
        DecodeStep4();
        break;
      }
      case 2: {
        DecodeStep5();
        break;
      }
      case 3: {
        DecodeStep6();
        break;
      }
      case 4: {
        DecodeStep7();
        break;
      }
      case 5: {
        DecodeStep8();
        break;
      }
      case 6: {
        DecodeStep12();
        break;
      }
      case 7: {
        DecodeStep9();
        break;
      }
      case 8: {
        DecodeStep10();
        break;
      }
      case 9: {
        DecodeStep11();
        break;
      }
      case 10: {
        DecodeStep13();
        break;
      }
    }
  }
  bool RefillTo15() {
    switch (buffer_len_) {
      case 0: {
        return Read2to8Bytes();
      }
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6: {
        return Read2to7Bytes();
      }
      case 7:
      case 8: {
        return Read1to7Bytes();
      }
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14: {
        return Read1to6Bytes();
      }
    }
    return true;
  }
  bool Read2to7Bytes() {
    switch (end_ - begin_) {
      case 0:
      case 1: {
        return false;
      }
      case 2: {
        Fill2();
        return true;
      }
      case 3: {
        Fill3();
        return true;
      }
      case 4: {
        Fill4();
        return true;
      }
      case 5: {
        Fill5();
        return true;
      }
      case 6: {
        Fill6();
        return true;
      }
      default: {
        Fill7();
        return true;
      }
    }
  }
  bool Read1to6Bytes() {
    switch (end_ - begin_) {
      case 0: {
        return false;
      }
      case 1: {
        Fill1();
        return true;
      }
      case 2: {
        Fill2();
        return true;
      }
      case 3: {
        Fill3();
        return true;
      }
      case 4: {
        Fill4();
        return true;
      }
      case 5: {
        Fill5();
        return true;
      }
      default: {
        Fill6();
        return true;
      }
    }
  }
  void Done4() {
    done_ = true;
    switch (end_ - begin_) {
      case 1: {
        Fill1();
        break;
      }
    }
    switch (buffer_len_) {
      case 1: {
        ok_ = (buffer_ & ((1 << buffer_len_) - 1)) == (1 << buffer_len_) - 1;
        return;
      }
      case 2: {
        const auto index = buffer_ & 3;
        const auto op = GetOp11(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit11(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 3: {
        const auto index = buffer_ & 7;
        const auto op = GetOp12(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit12(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 4: {
        const auto index = buffer_ & 15;
        const auto op = GetOp13(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit13(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 5: {
        const auto index = buffer_ & 31;
        const auto op = GetOp14(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit14(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 6: {
        const auto index = buffer_ & 63;
        const auto op = GetOp15(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit15(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 7: {
        const auto index = buffer_ & 127;
        const auto op = GetOp16(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit16(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 8: {
        const auto index = buffer_ & 255;
        const auto op = GetOp17(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit17(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 9: {
        const auto index = buffer_ & 511;
        const auto op = GetOp18(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit18(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 10: {
        const auto index = buffer_ & 1023;
        const auto op = GetOp19(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit19(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 11: {
        const auto index = buffer_ & 2047;
        const auto op = GetOp20(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit20(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 12: {
        const auto index = buffer_ & 4095;
        const auto op = GetOp21(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit21(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 13: {
        const auto index = buffer_ & 8191;
        const auto op = GetOp22(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit22(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 14: {
        const auto index = buffer_ & 16383;
        const auto op = GetOp23(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit23(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 0: {
        return;
      }
    }
  }
  void DecodeStep4() {
    if (!RefillTo1()) {
      Done5();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 1)) & 0x1;
    const auto op = GetOp24(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit24(index, emit_ofs + 0));
  }
  void Done5() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep5() {
    if (!RefillTo1()) {
      Done6();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 1)) & 0x1;
    const auto op = GetOp25(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit25(index, emit_ofs + 0));
  }
  void Done6() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep6() {
    if (!RefillTo2()) {
      Done7();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 2)) & 0x3;
    const auto op = GetOp26(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit26(index, emit_ofs + 0));
  }
  void Done7() {
    done_ = true;
    switch (buffer_len_) {
      case 1:
      case 0: {
        ok_ = false;
        return;
      }
    }
  }
  void DecodeStep7() {
    if (!RefillTo2()) {
      Done8();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 2)) & 0x3;
    const auto op = GetOp27(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit27(index, emit_ofs + 0));
  }
  void Done8() {
    done_ = true;
    switch (buffer_len_) {
      case 1:
      case 0: {
        ok_ = false;
        return;
      }
    }
  }
  void DecodeStep8() {
    if (!RefillTo2()) {
      Done9();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 2)) & 0x3;
    const auto op = GetOp28(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit28(index, emit_ofs + 0));
  }
  void Done9() {
    done_ = true;
    switch (buffer_len_) {
      case 1:
      case 0: {
        ok_ = false;
        return;
      }
    }
  }
  void DecodeStep9() {
    if (!RefillTo3()) {
      Done10();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 3)) & 0x7;
    const auto op = GetOp29(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit29(index, emit_ofs + 0));
  }
  bool RefillTo3() {
    switch (buffer_len_) {
      case 0: {
        return Read1to8Bytes();
      }
      case 1:
      case 2: {
        return Read1to7Bytes();
      }
    }
    return true;
  }
  void Done10() {
    done_ = true;
    switch (buffer_len_) {
      case 1:
      case 2:
      case 0: {
        ok_ = false;
        return;
      }
    }
  }
  void DecodeStep10() {
    if (!RefillTo3()) {
      Done11();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 3)) & 0x7;
    const auto op = GetOp30(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit30(index, emit_ofs + 0));
  }
  void Done11() {
    done_ = true;
    switch (buffer_len_) {
      case 1:
      case 2:
      case 0: {
        ok_ = false;
        return;
      }
    }
  }
  void DecodeStep11() {
    if (!RefillTo4()) {
      Done12();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 4)) & 0xf;
    const auto op = GetOp31(index);
    const int consumed = op & 7;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 3;
    sink_(GetEmit31(index, emit_ofs + 0));
  }
  bool RefillTo4() {
    switch (buffer_len_) {
      case 0: {
        return Read1to8Bytes();
      }
      case 1:
      case 2:
      case 3: {
        return Read1to7Bytes();
      }
    }
    return true;
  }
  void Done12() {
    done_ = true;
    switch (buffer_len_) {
      case 1:
      case 2:
      case 0: {
        ok_ = false;
        return;
      }
      case 3: {
        const auto index = buffer_ & 7;
        const auto op = GetOp32(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit32(index, op >> 2));
            break;
          }
        }
        return;
      }
    }
  }
  void DecodeStep12() {
    if (!RefillTo3()) {
      Done13();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 3)) & 0x7;
    const auto op = GetOp33(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit33(index, emit_ofs + 0));
  }
  void Done13() {
    done_ = true;
    switch (buffer_len_) {
      case 1:
      case 0: {
        ok_ = false;
        return;
      }
      case 2: {
        const auto index = buffer_ & 3;
        const auto op = GetOp34(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit34(index, op >> 2));
            break;
          }
        }
        return;
      }
    }
  }
  void DecodeStep13() {
    if (!RefillTo6()) {
      Done14();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 6)) & 0x3f;
    const auto op = GetOp35(index);
    const int consumed = op & 7;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 4;
    switch ((op >> 3) & 1) {
      case 0: {
        sink_(GetEmit35(index, emit_ofs + 0));
        break;
      }
      case 1: {
        begin_ = end_;
        buffer_len_ = 0;
        break;
      }
    }
  }
  bool RefillTo6() {
    switch (buffer_len_) {
      case 0: {
        return Read1to8Bytes();
      }
      case 1:
      case 2:
      case 3:
      case 4:
      case 5: {
        return Read1to7Bytes();
      }
    }
    return true;
  }
  void Done14() {
    done_ = true;
    switch (buffer_len_) {
      case 1:
      case 2:
      case 3: {
        ok_ = (buffer_ & ((1 << buffer_len_) - 1)) == (1 << buffer_len_) - 1;
        return;
      }
      case 4: {
        const auto index = buffer_ & 15;
        const auto op = GetOp36(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit36(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 5: {
        const auto index = buffer_ & 31;
        const auto op = GetOp37(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit37(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 0: {
        return;
      }
    }
  }
  F sink_;
  const uint8_t* begin_;
  const uint8_t* const end_;
  uint64_t buffer_ = 0;
  int buffer_len_ = 0;
  bool ok_ = true;
  bool done_ = false;
};
}  // namespace geometry_9_15_6
}  // namespace grpc_core
#endif  // GRPC_TEST_CPP_MICROBENCHMARKS_HUFFMAN_GEOMETRIES_DECODE_HUFF_9_15_6_H
