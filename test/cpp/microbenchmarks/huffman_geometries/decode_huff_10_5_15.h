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

#ifndef GRPC_TEST_CPP_MICROBENCHMARKS_HUFFMAN_GEOMETRIES_DECODE_HUFF_10_5_15_H
#define GRPC_TEST_CPP_MICROBENCHMARKS_HUFFMAN_GEOMETRIES_DECODE_HUFF_10_5_15_H
#include <grpc/support/port_platform.h>

#include <cstddef>
#include <cstdint>
namespace grpc_core {
namespace geometry_10_5_15 {
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
  static inline uint64_t GetOp6(size_t i) {
    return table6_ops_[i >> 5][i & 0x1f];
  }
  static inline uint64_t GetEmit6(size_t i, size_t emit) {
    return table6_emit_[i >> 5][emit];
  }
  static inline uint64_t GetOp1(size_t i) {
    return table1_inner_[i >> 6][table1_outer_[i >> 6][i & 0x3f]];
  }
  static inline uint64_t GetEmit1(size_t i, size_t emit) {
    return table1_emit_[i >> 6][emit];
  }
  static inline uint64_t GetOp7(size_t i) { return i ? 3 : 1; }
  static inline uint64_t GetEmit7(size_t, size_t emit) {
    return emit ? 43 : 39;
  }
  static inline uint64_t GetOp9(size_t i) { return table9_0_inner_[i]; }
  static inline uint64_t GetEmit9(size_t, size_t emit) {
    return table9_0_emit_[emit];
  }
  static inline uint64_t GetOp10(size_t i) {
    return table10_0_inner_[table10_0_outer_[i]];
  }
  static inline uint64_t GetEmit10(size_t, size_t emit) {
    return table10_0_emit_[emit];
  }
  static inline uint64_t GetOp8(size_t i) { return table8_0_ops_[i]; }
  static inline uint64_t GetEmit8(size_t, size_t emit) {
    return table8_0_emit_[emit];
  }
  static inline uint64_t GetOp12(size_t i) {
    return table12_0_inner_[(i < 3 ? (i) : ((i - 3) / 12 + 3))];
  }
  static inline uint64_t GetEmit12(size_t, size_t emit) {
    return (emit < 1 ? (((void)emit, 92)) : ((emit - 1) ? 208 : 195));
  }
  static inline uint64_t GetOp13(size_t i) { return table13_0_ops_[i]; }
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
  static inline uint64_t GetOp16(size_t i) {
    return table16_ops_[i >> 7][i & 0x7f];
  }
  static inline uint64_t GetEmit16(size_t i, size_t emit) {
    return table16_emit_[i >> 7][emit];
  }
  static inline uint64_t GetOp17(size_t i) {
    return table17_ops_[i >> 7][i & 0x7f];
  }
  static inline uint64_t GetEmit17(size_t i, size_t emit) {
    return table17_emit_[i >> 7][emit];
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
  static inline uint64_t GetOp11(size_t i) {
    return table11_ops_[i >> 8][i & 0xff];
  }
  static inline uint64_t GetEmit11(size_t i, size_t emit) {
    return table11_emit_[i >> 8][emit];
  }
  static inline uint64_t GetOp24(size_t i) { return i ? 1 : 2; }
  static inline uint64_t GetEmit24(size_t, size_t emit) {
    return ((void)emit, 124);
  }
  static inline uint64_t GetOp23(size_t i) {
    return ((i < 1 ? (((void)i, 0)) : ((i - 1))) < 1
                ? (((void)(i < 1 ? (((void)i, 0)) : ((i - 1))), 1))
                : (((i < 1 ? (((void)i, 0)) : ((i - 1))) - 1) ? 10 : 6));
  }
  static inline uint64_t GetEmit23(size_t, size_t emit) {
    return (emit < 1 ? (((void)emit, 124)) : ((emit - 1) ? 62 : 35));
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
  static const uint8_t table6_0_emit_[2];
  static const uint8_t table6_0_ops_[32];
  static const uint8_t table6_1_emit_[2];
  static const uint8_t table6_2_emit_[2];
  static const uint8_t table6_3_emit_[2];
  static const uint8_t table6_4_emit_[2];
  static const uint8_t table6_5_emit_[4];
  static const uint8_t table6_5_ops_[32];
  static const uint8_t table6_6_emit_[4];
  static const uint8_t table6_7_emit_[4];
  static const uint8_t table6_8_emit_[4];
  static const uint8_t table6_9_emit_[4];
  static const uint8_t table6_10_emit_[4];
  static const uint8_t table6_11_emit_[6];
  static const uint8_t table6_11_ops_[32];
  static const uint8_t table6_12_emit_[8];
  static const uint8_t table6_12_ops_[32];
  static const uint8_t table6_13_emit_[8];
  static const uint8_t table6_14_emit_[8];
  static const uint8_t table6_15_emit_[10];
  static const uint8_t table6_15_ops_[32];
  static const uint8_t* const table6_emit_[16];
  static const uint8_t* const table6_ops_[16];
  static const uint8_t table1_0_emit_[36];
  static const uint16_t table1_0_inner_[22];
  static const uint8_t table1_0_outer_[64];
  static const uint8_t table1_1_emit_[36];
  static const uint16_t table1_1_inner_[22];
  static const uint8_t table1_2_emit_[36];
  static const uint16_t table1_2_inner_[22];
  static const uint8_t table1_3_emit_[36];
  static const uint16_t table1_3_inner_[22];
  static const uint8_t table1_4_emit_[38];
  static const uint16_t table1_4_inner_[22];
  static const uint16_t table1_5_inner_[4];
  static const uint8_t table1_5_outer_[64];
  static const uint16_t table1_11_inner_[6];
  static const uint8_t table1_11_outer_[64];
  static const uint16_t table1_12_inner_[8];
  static const uint8_t table1_12_outer_[64];
  static const uint8_t table1_15_emit_[15];
  static const uint16_t table1_15_inner_[18];
  static const uint8_t table1_15_outer_[64];
  static const uint8_t* const table1_emit_[16];
  static const uint16_t* const table1_inner_[16];
  static const uint8_t* const table1_outer_[16];
  static const uint8_t table9_0_emit_[6];
  static const uint8_t table9_0_inner_[8];
  static const uint8_t table10_0_emit_[8];
  static const uint8_t table10_0_inner_[10];
  static const uint8_t table10_0_outer_[16];
  static const uint8_t table8_0_emit_[11];
  static const uint8_t table8_0_ops_[32];
  static const uint8_t table12_0_inner_[5];
  static const uint8_t table13_0_emit_[11];
  static const uint8_t table13_0_ops_[32];
  static const uint8_t table14_0_emit_[24];
  static const uint8_t table14_0_ops_[64];
  static const uint8_t table15_0_emit_[50];
  static const uint8_t table15_0_ops_[128];
  static const uint8_t table16_0_emit_[15];
  static const uint8_t table16_0_ops_[128];
  static const uint8_t table16_1_emit_[64];
  static const uint8_t table16_1_ops_[128];
  static const uint8_t* const table16_emit_[2];
  static const uint8_t* const table16_ops_[2];
  static const uint8_t table17_0_emit_[5];
  static const uint8_t table17_0_ops_[128];
  static const uint8_t table17_1_emit_[10];
  static const uint8_t table17_1_ops_[128];
  static const uint8_t table17_2_emit_[23];
  static const uint8_t table17_2_ops_[128];
  static const uint8_t table17_3_emit_[53];
  static const uint8_t table17_3_ops_[128];
  static const uint8_t* const table17_emit_[4];
  static const uint8_t* const table17_ops_[4];
  static const uint8_t table18_0_emit_[1];
  static const uint8_t table18_0_ops_[64];
  static const uint8_t table18_1_emit_[1];
  static const uint8_t table18_2_emit_[1];
  static const uint8_t table18_3_emit_[2];
  static const uint8_t table18_3_ops_[64];
  static const uint8_t table18_4_emit_[2];
  static const uint8_t table18_5_emit_[2];
  static const uint8_t table18_6_emit_[2];
  static const uint8_t table18_7_emit_[4];
  static const uint8_t table18_7_ops_[64];
  static const uint8_t table18_8_emit_[4];
  static const uint8_t table18_9_emit_[4];
  static const uint8_t table18_10_emit_[7];
  static const uint8_t table18_10_ops_[64];
  static const uint8_t table18_11_emit_[8];
  static const uint8_t table18_11_ops_[64];
  static const uint8_t table18_12_emit_[8];
  static const uint8_t table18_13_emit_[12];
  static const uint8_t table18_13_ops_[64];
  static const uint8_t table18_14_emit_[16];
  static const uint8_t table18_14_ops_[64];
  static const uint8_t table18_15_emit_[21];
  static const uint8_t table18_15_ops_[64];
  static const uint8_t* const table18_emit_[16];
  static const uint8_t* const table18_ops_[16];
  static const uint8_t table19_0_emit_[0];
  static const uint8_t table19_0_ops_[64];
  static const uint8_t table19_6_emit_[1];
  static const uint8_t table19_7_emit_[1];
  static const uint8_t table19_8_emit_[1];
  static const uint8_t table19_9_emit_[1];
  static const uint8_t table19_10_emit_[1];
  static const uint8_t table19_11_emit_[1];
  static const uint8_t table19_12_emit_[1];
  static const uint8_t table19_13_emit_[1];
  static const uint8_t table19_14_emit_[2];
  static const uint8_t table19_15_emit_[2];
  static const uint8_t table19_16_emit_[2];
  static const uint8_t table19_17_emit_[2];
  static const uint8_t table19_18_emit_[2];
  static const uint8_t table19_19_emit_[2];
  static const uint8_t table19_20_emit_[3];
  static const uint8_t table19_20_ops_[64];
  static const uint8_t table19_21_emit_[4];
  static const uint8_t table19_22_emit_[4];
  static const uint8_t table19_23_emit_[4];
  static const uint8_t table19_24_emit_[4];
  static const uint8_t table19_25_emit_[4];
  static const uint8_t table19_26_emit_[4];
  static const uint8_t table19_27_emit_[8];
  static const uint8_t table19_28_emit_[8];
  static const uint8_t table19_29_emit_[8];
  static const uint8_t table19_30_emit_[11];
  static const uint8_t table19_30_ops_[64];
  static const uint8_t table19_31_emit_[25];
  static const uint8_t table19_31_ops_[64];
  static const uint8_t* const table19_emit_[32];
  static const uint8_t* const table19_ops_[32];
  static const uint8_t table20_0_ops_[128];
  static const uint8_t table20_1_ops_[128];
  static const uint8_t table20_14_ops_[128];
  static const uint8_t table20_20_ops_[128];
  static const uint8_t table20_21_ops_[128];
  static const uint8_t table20_27_ops_[128];
  static const uint8_t table20_30_ops_[128];
  static const uint8_t table20_31_emit_[44];
  static const uint8_t table20_31_ops_[128];
  static const uint8_t* const table20_emit_[32];
  static const uint8_t* const table20_ops_[32];
  static const uint8_t table21_28_emit_[1];
  static const uint8_t table21_29_emit_[1];
  static const uint8_t table21_30_emit_[1];
  static const uint8_t table21_31_emit_[1];
  static const uint8_t table21_32_emit_[1];
  static const uint8_t table21_33_emit_[1];
  static const uint8_t table21_34_emit_[1];
  static const uint8_t table21_35_emit_[1];
  static const uint8_t table21_36_emit_[1];
  static const uint8_t table21_37_emit_[1];
  static const uint8_t table21_38_emit_[1];
  static const uint8_t table21_39_emit_[1];
  static const uint8_t table21_40_emit_[1];
  static const uint8_t table21_41_emit_[2];
  static const uint8_t table21_42_emit_[2];
  static const uint8_t table21_43_emit_[2];
  static const uint8_t table21_44_emit_[2];
  static const uint8_t table21_45_emit_[2];
  static const uint8_t table21_46_emit_[2];
  static const uint8_t table21_47_emit_[2];
  static const uint8_t table21_48_emit_[2];
  static const uint8_t table21_49_emit_[2];
  static const uint8_t table21_50_emit_[2];
  static const uint8_t table21_51_emit_[2];
  static const uint8_t table21_52_emit_[2];
  static const uint8_t table21_53_emit_[2];
  static const uint8_t table21_54_emit_[4];
  static const uint8_t table21_55_emit_[4];
  static const uint8_t table21_56_emit_[4];
  static const uint8_t table21_57_emit_[4];
  static const uint8_t table21_58_emit_[4];
  static const uint8_t table21_59_emit_[4];
  static const uint8_t table21_60_emit_[4];
  static const uint8_t table21_61_emit_[7];
  static const uint8_t table21_61_ops_[128];
  static const uint8_t table21_62_emit_[10];
  static const uint8_t table21_63_emit_[63];
  static const uint8_t table21_63_ops_[128];
  static const uint8_t* const table21_emit_[64];
  static const uint8_t* const table21_ops_[64];
  static const uint8_t table22_0_ops_[256];
  static const uint8_t table22_3_ops_[256];
  static const uint8_t table22_41_ops_[256];
  static const uint8_t table22_54_ops_[256];
  static const uint8_t table22_61_ops_[256];
  static const uint8_t table22_62_ops_[256];
  static const uint8_t table22_63_ops_[256];
  static const uint8_t* const table22_emit_[64];
  static const uint8_t* const table22_ops_[64];
  static const uint16_t table11_0_ops_[256];
  static const uint16_t table11_24_ops_[256];
  static const uint16_t table11_56_ops_[256];
  static const uint8_t table11_82_emit_[1];
  static const uint16_t table11_82_ops_[256];
  static const uint8_t table11_83_emit_[1];
  static const uint8_t table11_84_emit_[1];
  static const uint8_t table11_85_emit_[1];
  static const uint8_t table11_86_emit_[1];
  static const uint8_t table11_87_emit_[1];
  static const uint8_t table11_88_emit_[1];
  static const uint8_t table11_89_emit_[1];
  static const uint8_t table11_90_emit_[1];
  static const uint8_t table11_91_emit_[1];
  static const uint8_t table11_92_emit_[1];
  static const uint8_t table11_93_emit_[1];
  static const uint8_t table11_94_emit_[1];
  static const uint8_t table11_95_emit_[1];
  static const uint8_t table11_96_emit_[1];
  static const uint8_t table11_97_emit_[1];
  static const uint8_t table11_98_emit_[1];
  static const uint8_t table11_99_emit_[1];
  static const uint8_t table11_100_emit_[1];
  static const uint8_t table11_101_emit_[1];
  static const uint8_t table11_102_emit_[1];
  static const uint8_t table11_103_emit_[1];
  static const uint8_t table11_104_emit_[1];
  static const uint8_t table11_105_emit_[1];
  static const uint8_t table11_106_emit_[1];
  static const uint8_t table11_107_emit_[1];
  static const uint8_t table11_108_emit_[2];
  static const uint16_t table11_108_ops_[256];
  static const uint8_t table11_109_emit_[2];
  static const uint8_t table11_110_emit_[2];
  static const uint8_t table11_111_emit_[2];
  static const uint8_t table11_112_emit_[2];
  static const uint8_t table11_113_emit_[2];
  static const uint8_t table11_114_emit_[2];
  static const uint8_t table11_115_emit_[2];
  static const uint8_t table11_116_emit_[2];
  static const uint8_t table11_117_emit_[2];
  static const uint8_t table11_118_emit_[2];
  static const uint8_t table11_119_emit_[2];
  static const uint8_t table11_120_emit_[2];
  static const uint8_t table11_121_emit_[2];
  static const uint8_t table11_122_emit_[3];
  static const uint16_t table11_122_ops_[256];
  static const uint8_t table11_123_emit_[4];
  static const uint16_t table11_123_ops_[256];
  static const uint8_t table11_124_emit_[4];
  static const uint8_t table11_125_emit_[6];
  static const uint16_t table11_125_ops_[256];
  static const uint8_t table11_126_emit_[17];
  static const uint16_t table11_126_ops_[256];
  static const uint8_t table11_127_emit_[49];
  static const uint16_t table11_127_ops_[256];
  static const uint8_t* const table11_emit_[128];
  static const uint16_t* const table11_ops_[128];
};
template <typename F>
class HuffDecoder : public HuffDecoderCommon {
 public:
  HuffDecoder(F sink, const uint8_t* begin, const uint8_t* end)
      : sink_(sink), begin_(begin), end_(end) {}
  bool Run() {
    while (!done_) {
      if (!RefillTo10()) {
        Done0();
        break;
      }
      const auto index = (buffer_ >> (buffer_len_ - 10)) & 0x3ff;
      const auto op = GetOp1(index);
      const int consumed = op & 15;
      buffer_len_ -= consumed;
      const auto emit_ofs = op >> 7;
      switch ((op >> 4) & 7) {
        case 2: {
          DecodeStep0();
          break;
        }
        case 4: {
          DecodeStep1();
          break;
        }
        case 3: {
          DecodeStep3();
          break;
        }
        case 1: {
          sink_(GetEmit1(index, emit_ofs + 0));
          break;
        }
        case 0: {
          sink_(GetEmit1(index, emit_ofs + 0));
          sink_(GetEmit1(index, emit_ofs + 1));
          break;
        }
      }
    }
    return ok_;
  }

 private:
  bool RefillTo10() {
    switch (buffer_len_) {
      case 9: {
        return Read1to6Bytes();
      }
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8: {
        return Read1to7Bytes();
      }
      case 1: {
        return Read2to7Bytes();
      }
      case 0: {
        return Read2to8Bytes();
      }
    }
    return true;
  }
  bool Read2to8Bytes() {
    switch (end_ - begin_) {
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
      case 0:
      case 1: {
        return false;
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
  bool Read2to7Bytes() {
    switch (end_ - begin_) {
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
      case 0:
      case 1: {
        return false;
      }
    }
  }
  bool Read1to7Bytes() {
    switch (end_ - begin_) {
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
      case 0: {
        return false;
      }
    }
  }
  void Fill1() {
    buffer_ = (buffer_ << 8) | (static_cast<uint64_t>(begin_[0]) << 0);
    begin_ += 1;
    buffer_len_ += 8;
  }
  bool Read1to6Bytes() {
    switch (end_ - begin_) {
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
      case 0: {
        return false;
      }
    }
  }
  void Done0() {
    done_ = true;
    switch (buffer_len_) {
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
      case 9: {
        const auto index = buffer_ & 511;
        const auto op = GetOp6(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit6(index, op >> 2));
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
      case 1:
      case 2:
      case 3:
      case 4: {
        ok_ = (buffer_ & ((1 << buffer_len_) - 1)) == (1 << buffer_len_) - 1;
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
    const auto op = GetOp7(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit7(index, emit_ofs + 0));
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
      case 0: {
        return false;
      }
    }
  }
  void Done1() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep1() {
    if (!RefillTo5()) {
      Done2();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 5)) & 0x1f;
    const auto op = GetOp8(index);
    const int consumed = op & 7;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 4;
    switch ((op >> 3) & 1) {
      case 1: {
        DecodeStep2();
        break;
      }
      case 0: {
        sink_(GetEmit8(index, emit_ofs + 0));
        break;
      }
    }
  }
  bool RefillTo5() {
    switch (buffer_len_) {
      case 1:
      case 2:
      case 3:
      case 4: {
        return Read1to7Bytes();
      }
      case 0: {
        return Read1to8Bytes();
      }
    }
    return true;
  }
  void Done2() {
    done_ = true;
    switch (buffer_len_) {
      case 4: {
        const auto index = buffer_ & 15;
        const auto op = GetOp10(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit10(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 3: {
        const auto index = buffer_ & 7;
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
      case 1:
      case 2: {
        ok_ = (buffer_ & ((1 << buffer_len_) - 1)) == (1 << buffer_len_) - 1;
        return;
      }
      case 0: {
        return;
      }
    }
  }
  void DecodeStep2() {
    if (!RefillTo15()) {
      Done3();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 15)) & 0x7fff;
    const auto op = GetOp11(index);
    const int consumed = op & 15;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 5;
    switch ((op >> 4) & 1) {
      case 1: {
        begin_ = end_;
        buffer_len_ = 0;
        break;
      }
      case 0: {
        sink_(GetEmit11(index, emit_ofs + 0));
        break;
      }
    }
  }
  bool RefillTo15() {
    switch (buffer_len_) {
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 9: {
        return Read1to6Bytes();
      }
      case 7:
      case 8: {
        return Read1to7Bytes();
      }
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6: {
        return Read2to7Bytes();
      }
      case 0: {
        return Read2to8Bytes();
      }
    }
    return true;
  }
  void Done3() {
    done_ = true;
    switch (buffer_len_) {
      case 10: {
        const auto index = buffer_ & 1023;
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
      case 7: {
        const auto index = buffer_ & 127;
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
      case 4: {
        const auto index = buffer_ & 15;
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
      case 14: {
        const auto index = buffer_ & 16383;
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
      case 11: {
        const auto index = buffer_ & 2047;
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
      case 8: {
        const auto index = buffer_ & 255;
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
      case 5: {
        const auto index = buffer_ & 31;
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
      case 12: {
        const auto index = buffer_ & 4095;
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
      case 9: {
        const auto index = buffer_ & 511;
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
      case 6: {
        const auto index = buffer_ & 63;
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
      case 13: {
        const auto index = buffer_ & 8191;
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
      case 1:
      case 2:
      case 3: {
        ok_ = (buffer_ & ((1 << buffer_len_) - 1)) == (1 << buffer_len_) - 1;
        return;
      }
      case 0: {
        return;
      }
    }
  }
  void DecodeStep3() {
    if (!RefillTo2()) {
      Done4();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 2)) & 0x3;
    const auto op = GetOp23(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit23(index, emit_ofs + 0));
  }
  bool RefillTo2() {
    switch (buffer_len_) {
      case 1: {
        return Read1to7Bytes();
      }
      case 0: {
        return Read1to8Bytes();
      }
    }
    return true;
  }
  void Done4() {
    done_ = true;
    switch (buffer_len_) {
      case 1: {
        const auto index = buffer_ & 1;
        const auto op = GetOp24(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit24(index, op >> 2));
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
  F sink_;
  const uint8_t* begin_;
  const uint8_t* const end_;
  uint64_t buffer_ = 0;
  int buffer_len_ = 0;
  bool ok_ = true;
  bool done_ = false;
};
}  // namespace geometry_10_5_15
}  // namespace grpc_core
#endif  // GRPC_TEST_CPP_MICROBENCHMARKS_HUFFMAN_GEOMETRIES_DECODE_HUFF_10_5_15_H
