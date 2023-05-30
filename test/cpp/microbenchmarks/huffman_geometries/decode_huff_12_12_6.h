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

#ifndef GRPC_TEST_CPP_MICROBENCHMARKS_HUFFMAN_GEOMETRIES_DECODE_HUFF_12_12_6_H
#define GRPC_TEST_CPP_MICROBENCHMARKS_HUFFMAN_GEOMETRIES_DECODE_HUFF_12_12_6_H
#include <grpc/support/port_platform.h>

#include <cstddef>
#include <cstdint>
namespace grpc_core {
namespace geometry_12_12_6 {
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
  static inline uint64_t GetOp7(size_t i) {
    return table7_ops_[i >> 6][i & 0x3f];
  }
  static inline uint64_t GetEmit7(size_t i, size_t emit) {
    return table7_emit_[i >> 6][emit];
  }
  static inline uint64_t GetOp8(size_t i) {
    return table8_ops_[i >> 7][i & 0x7f];
  }
  static inline uint64_t GetEmit8(size_t i, size_t emit) {
    return table8_emit_[i >> 7][emit];
  }
  static inline uint64_t GetOp1(size_t i) {
    return table1_inner_[i >> 7][table1_outer_[i >> 7][i & 0x7f]];
  }
  static inline uint64_t GetEmit1(size_t i, size_t emit) {
    return table1_emit_[i >> 7][emit];
  }
  static inline uint64_t GetOp9(size_t i) { return i ? 3 : 1; }
  static inline uint64_t GetEmit9(size_t, size_t emit) { return emit ? 36 : 0; }
  static inline uint64_t GetOp10(size_t i) { return i ? 3 : 1; }
  static inline uint64_t GetEmit10(size_t, size_t emit) {
    return emit ? 91 : 64;
  }
  static inline uint64_t GetOp11(size_t i) { return i ? 3 : 1; }
  static inline uint64_t GetEmit11(size_t, size_t emit) {
    return emit ? 126 : 93;
  }
  static inline uint64_t GetOp13(size_t i) {
    return (i < 2 ? (i ? 6 : 2) : ((i - 2) ? 0 : 1));
  }
  static inline uint64_t GetEmit13(size_t, size_t emit) {
    return emit ? 125 : 94;
  }
  static inline uint64_t GetOp14(size_t i) {
    return table14_0_inner_[table14_0_outer_[i]];
  }
  static inline uint64_t GetEmit14(size_t, size_t emit) {
    return table14_0_emit_[emit];
  }
  static inline uint64_t GetOp15(size_t i) {
    return table14_0_inner_[table15_0_outer_[i]];
  }
  static inline uint64_t GetEmit15(size_t, size_t emit) {
    return table14_0_emit_[emit];
  }
  static inline uint64_t GetOp16(size_t i) { return table16_0_ops_[i]; }
  static inline uint64_t GetEmit16(size_t, size_t emit) {
    return table14_0_emit_[emit];
  }
  static inline uint64_t GetOp17(size_t i) { return table17_0_ops_[i]; }
  static inline uint64_t GetEmit17(size_t, size_t emit) {
    return table14_0_emit_[emit];
  }
  static inline uint64_t GetOp18(size_t i) { return table18_0_ops_[i]; }
  static inline uint64_t GetEmit18(size_t, size_t emit) {
    return table18_0_emit_[emit];
  }
  static inline uint64_t GetOp19(size_t i) { return table19_0_ops_[i]; }
  static inline uint64_t GetEmit19(size_t, size_t emit) {
    return table19_0_emit_[emit];
  }
  static inline uint64_t GetOp20(size_t i) {
    return table20_ops_[i >> 6][i & 0x3f];
  }
  static inline uint64_t GetEmit20(size_t i, size_t emit) {
    return table20_emit_[i >> 6][emit];
  }
  static inline uint64_t GetOp21(size_t i) {
    return table21_ops_[i >> 7][i & 0x7f];
  }
  static inline uint64_t GetEmit21(size_t i, size_t emit) {
    return table21_emit_[i >> 7][emit];
  }
  static inline uint64_t GetOp22(size_t i) {
    return table22_ops_[i >> 7][i & 0x7f];
  }
  static inline uint64_t GetEmit22(size_t i, size_t emit) {
    return table22_emit_[i >> 7][emit];
  }
  static inline uint64_t GetOp12(size_t i) {
    return table12_ops_[i >> 8][i & 0xff];
  }
  static inline uint64_t GetEmit12(size_t i, size_t emit) {
    return table12_emit_[i >> 8][emit];
  }
  static inline uint64_t GetOp23(size_t i) { return i ? 3 : 1; }
  static inline uint64_t GetEmit23(size_t, size_t emit) {
    return emit ? 207 : 199;
  }
  static inline uint64_t GetOp24(size_t i) { return i ? 3 : 1; }
  static inline uint64_t GetEmit24(size_t, size_t emit) { return emit + 234; }
  static inline uint64_t GetOp25(size_t i) {
    return (i < 2 ? (i ? 6 : 2) : ((i - 2) ? 14 : 10));
  }
  static inline uint64_t GetEmit25(size_t, size_t emit) {
    return (emit < 2 ? (emit + 192) : ((emit - 2) + 200));
  }
  static inline uint64_t GetOp26(size_t i) {
    return (i < 2 ? (i ? 6 : 2) : ((i - 2) ? 14 : 10));
  }
  static inline uint64_t GetEmit26(size_t, size_t emit) {
    return (emit < 2 ? (emit ? 205 : 202) : ((emit - 2) ? 213 : 210));
  }
  static inline uint64_t GetOp27(size_t i) {
    return (i < 2 ? (i ? 6 : 2) : ((i - 2) ? 14 : 10));
  }
  static inline uint64_t GetEmit27(size_t, size_t emit) {
    return (emit < 2 ? (emit + 218) : ((emit - 2) ? 240 : 238));
  }
  static inline uint64_t GetOp28(size_t i) { return table28_0_inner_[i]; }
  static inline uint64_t GetEmit28(size_t, size_t emit) {
    return table28_0_emit_[emit];
  }
  static inline uint64_t GetOp29(size_t i) { return table28_0_inner_[i]; }
  static inline uint64_t GetEmit29(size_t, size_t emit) {
    return (emit < 4 ? (emit + 245) : ((emit - 4) + 250));
  }
  static inline uint64_t GetOp31(size_t i) {
    return (i < 1 ? (((void)i, 0)) : (((void)(i - 1), 1))) ? 1 : 2;
  }
  static inline uint64_t GetEmit31(size_t, size_t emit) {
    return ((void)emit, 254);
  }
  static inline uint64_t GetOp30(size_t i) {
    return table30_0_inner_[(i < 1 ? (((void)i, 0)) : ((i - 1)))];
  }
  static inline uint64_t GetEmit30(size_t, size_t emit) {
    return table30_0_emit_[emit];
  }
  static inline uint64_t GetOp33(size_t i) {
    return (i < 2 ? (i ? 6 : 2) : ((i - 2) ? 1 : 10));
  }
  static inline uint64_t GetEmit33(size_t, size_t emit) {
    return (emit < 1 ? (((void)emit, 242)) : ((emit - 1) ? 255 : 243));
  }
  static inline uint64_t GetOp32(size_t i) {
    return table32_0_inner_[(i < 5 ? (i / 2 + 0) : ((i - 5) + 2))];
  }
  static inline uint64_t GetEmit32(size_t, size_t emit) {
    return table32_0_emit_[emit];
  }
  static inline uint64_t GetOp35(size_t i) { return table35_0_inner_[i]; }
  static inline uint64_t GetEmit35(size_t, size_t emit) {
    return table35_0_emit_[emit];
  }
  static inline uint64_t GetOp36(size_t i) { return table36_0_ops_[i]; }
  static inline uint64_t GetEmit36(size_t, size_t emit) {
    return table35_0_emit_[emit];
  }
  static inline uint64_t GetOp34(size_t i) {
    return table34_ops_[i >> 5][i & 0x1f];
  }
  static inline uint64_t GetEmit34(size_t i, size_t emit) {
    return table34_emit_[i >> 5][emit];
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
  static const uint8_t table7_0_ops_[64];
  static const uint8_t table7_5_ops_[64];
  static const uint8_t table7_11_ops_[64];
  static const uint8_t table7_12_ops_[64];
  static const uint8_t table7_15_emit_[15];
  static const uint8_t table7_15_ops_[64];
  static const uint8_t* const table7_emit_[16];
  static const uint8_t* const table7_ops_[16];
  static const uint8_t table8_0_ops_[128];
  static const uint8_t table8_5_ops_[128];
  static const uint8_t table8_11_ops_[128];
  static const uint8_t table8_12_ops_[128];
  static const uint8_t table8_15_emit_[18];
  static const uint8_t table8_15_ops_[128];
  static const uint8_t* const table8_emit_[16];
  static const uint8_t* const table8_ops_[16];
  static const uint8_t table1_0_emit_[135];
  static const uint16_t table1_0_inner_[69];
  static const uint8_t table1_0_outer_[128];
  static const uint8_t table1_1_emit_[135];
  static const uint16_t table1_1_inner_[69];
  static const uint8_t table1_2_emit_[135];
  static const uint16_t table1_2_inner_[69];
  static const uint8_t table1_3_emit_[135];
  static const uint16_t table1_3_inner_[69];
  static const uint8_t table1_4_emit_[135];
  static const uint16_t table1_4_inner_[69];
  static const uint8_t table1_5_emit_[135];
  static const uint16_t table1_5_inner_[69];
  static const uint8_t table1_6_emit_[135];
  static const uint16_t table1_6_inner_[69];
  static const uint8_t table1_7_emit_[135];
  static const uint16_t table1_7_inner_[69];
  static const uint8_t table1_8_emit_[135];
  static const uint16_t table1_8_inner_[69];
  static const uint8_t table1_9_emit_[135];
  static const uint16_t table1_9_inner_[69];
  static const uint8_t table1_10_emit_[140];
  static const uint16_t table1_10_inner_[74];
  static const uint8_t table1_10_outer_[128];
  static const uint8_t table1_11_emit_[140];
  static const uint16_t table1_11_inner_[74];
  static const uint8_t table1_12_emit_[140];
  static const uint16_t table1_12_inner_[74];
  static const uint8_t table1_13_emit_[140];
  static const uint16_t table1_13_inner_[74];
  static const uint8_t table1_14_emit_[140];
  static const uint16_t table1_14_inner_[74];
  static const uint8_t table1_15_emit_[140];
  static const uint16_t table1_15_inner_[74];
  static const uint8_t table1_16_emit_[140];
  static const uint16_t table1_16_inner_[74];
  static const uint8_t table1_17_emit_[140];
  static const uint16_t table1_17_inner_[74];
  static const uint8_t table1_18_emit_[140];
  static const uint16_t table1_18_inner_[74];
  static const uint8_t table1_19_emit_[140];
  static const uint16_t table1_19_inner_[74];
  static const uint8_t table1_20_emit_[140];
  static const uint16_t table1_20_inner_[74];
  static const uint8_t table1_21_emit_[140];
  static const uint16_t table1_21_inner_[74];
  static const uint8_t table1_22_emit_[142];
  static const uint16_t table1_22_inner_[74];
  static const uint8_t table1_23_emit_[80];
  static const uint16_t table1_23_inner_[44];
  static const uint8_t table1_23_outer_[128];
  static const uint8_t table1_24_emit_[80];
  static const uint8_t table1_25_emit_[80];
  static const uint8_t table1_26_emit_[80];
  static const uint8_t table1_27_emit_[80];
  static const uint8_t table1_28_emit_[80];
  static const uint8_t table1_29_emit_[80];
  static const uint8_t table1_30_emit_[80];
  static const uint8_t table1_31_emit_[16];
  static const uint16_t table1_31_inner_[20];
  static const uint8_t table1_31_outer_[128];
  static const uint8_t* const table1_emit_[32];
  static const uint16_t* const table1_inner_[32];
  static const uint8_t* const table1_outer_[32];
  static const uint8_t table14_0_emit_[5];
  static const uint8_t table14_0_inner_[7];
  static const uint8_t table14_0_outer_[8];
  static const uint8_t table15_0_outer_[16];
  static const uint8_t table16_0_ops_[32];
  static const uint8_t table17_0_ops_[64];
  static const uint8_t table18_0_emit_[8];
  static const uint8_t table18_0_ops_[128];
  static const uint8_t table19_0_emit_[16];
  static const uint8_t table19_0_ops_[256];
  static const uint8_t table20_0_ops_[64];
  static const uint8_t table20_1_emit_[1];
  static const uint8_t table20_1_ops_[64];
  static const uint8_t table20_3_emit_[1];
  static const uint8_t table20_4_emit_[1];
  static const uint8_t table20_5_emit_[1];
  static const uint8_t table20_6_emit_[1];
  static const uint8_t table20_7_emit_[24];
  static const uint8_t table20_7_ops_[64];
  static const uint8_t* const table20_emit_[8];
  static const uint8_t* const table20_ops_[8];
  static const uint8_t table21_0_ops_[128];
  static const uint8_t table21_1_ops_[128];
  static const uint8_t table21_7_emit_[50];
  static const uint8_t table21_7_ops_[128];
  static const uint8_t* const table21_emit_[8];
  static const uint8_t* const table21_ops_[8];
  static const uint8_t table22_14_emit_[15];
  static const uint8_t table22_14_ops_[128];
  static const uint8_t table22_15_emit_[64];
  static const uint8_t table22_15_ops_[128];
  static const uint8_t* const table22_emit_[16];
  static const uint8_t* const table22_ops_[16];
  static const uint16_t table12_0_ops_[256];
  static const uint16_t table12_8_ops_[256];
  static const uint16_t table12_14_ops_[256];
  static const uint8_t table12_15_emit_[76];
  static const uint16_t table12_15_ops_[256];
  static const uint8_t* const table12_emit_[16];
  static const uint16_t* const table12_ops_[16];
  static const uint8_t table28_0_emit_[8];
  static const uint8_t table28_0_inner_[8];
  static const uint8_t table30_0_emit_[15];
  static const uint8_t table30_0_inner_[15];
  static const uint8_t table32_0_emit_[5];
  static const uint8_t table32_0_inner_[5];
  static const uint8_t table35_0_emit_[15];
  static const uint8_t table35_0_inner_[16];
  static const uint8_t table36_0_ops_[32];
  static const uint8_t table34_0_emit_[8];
  static const uint8_t table34_0_ops_[32];
  static const uint8_t table34_1_emit_[10];
  static const uint8_t table34_1_ops_[32];
  static const uint8_t* const table34_emit_[2];
  static const uint8_t* const table34_ops_[2];
};
template <typename F>
class HuffDecoder : public HuffDecoderCommon {
 public:
  HuffDecoder(F sink, const uint8_t* begin, const uint8_t* end)
      : sink_(sink), begin_(begin), end_(end) {}
  bool Run() {
    while (!done_) {
      if (!RefillTo12()) {
        Done0();
        break;
      }
      const auto index = (buffer_ >> (buffer_len_ - 12)) & 0xfff;
      const auto op = GetOp1(index);
      const int consumed = op & 15;
      buffer_len_ -= consumed;
      const auto emit_ofs = op >> 7;
      switch ((op >> 4) & 7) {
        case 0: {
          sink_(GetEmit1(index, emit_ofs + 0));
          sink_(GetEmit1(index, emit_ofs + 1));
          break;
        }
        case 1: {
          sink_(GetEmit1(index, emit_ofs + 0));
          break;
        }
        case 2: {
          DecodeStep0();
          break;
        }
        case 3: {
          DecodeStep1();
          break;
        }
        case 4: {
          DecodeStep2();
          break;
        }
        case 5: {
          DecodeStep3();
          break;
        }
      }
    }
    return ok_;
  }

 private:
  bool RefillTo12() {
    switch (buffer_len_) {
      case 0: {
        return Read2to8Bytes();
      }
      case 1:
      case 2:
      case 3: {
        return Read2to7Bytes();
      }
      case 4:
      case 5:
      case 6:
      case 7:
      case 8: {
        return Read1to7Bytes();
      }
      case 9:
      case 10:
      case 11: {
        return Read1to6Bytes();
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
      case 10: {
        const auto index = buffer_ & 1023;
        const auto op = GetOp7(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit7(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 11: {
        const auto index = buffer_ & 2047;
        const auto op = GetOp8(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit8(index, op >> 2));
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
    const auto op = GetOp9(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit9(index, emit_ofs + 0));
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
    const auto op = GetOp10(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit10(index, emit_ofs + 0));
  }
  void Done2() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep2() {
    if (!RefillTo1()) {
      Done3();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 1)) & 0x1;
    const auto op = GetOp11(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit11(index, emit_ofs + 0));
  }
  void Done3() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep3() {
    if (!RefillTo12()) {
      Done4();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 12)) & 0xfff;
    const auto op = GetOp12(index);
    const int consumed = op & 15;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 8;
    switch ((op >> 4) & 15) {
      case 0: {
        sink_(GetEmit12(index, emit_ofs + 0));
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
      case 3: {
        const auto index = buffer_ & 7;
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
      case 4: {
        const auto index = buffer_ & 15;
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
      case 5: {
        const auto index = buffer_ & 31;
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
      case 6: {
        const auto index = buffer_ & 63;
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
      case 7: {
        const auto index = buffer_ & 127;
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
      case 8: {
        const auto index = buffer_ & 255;
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
      case 9: {
        const auto index = buffer_ & 511;
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
      case 10: {
        const auto index = buffer_ & 1023;
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
      case 11: {
        const auto index = buffer_ & 2047;
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
    const auto op = GetOp23(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit23(index, emit_ofs + 0));
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
    const auto op = GetOp24(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit24(index, emit_ofs + 0));
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
    const auto op = GetOp25(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit25(index, emit_ofs + 0));
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
    const auto op = GetOp26(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit26(index, emit_ofs + 0));
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
    const auto op = GetOp27(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit27(index, emit_ofs + 0));
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
    const auto op = GetOp28(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit28(index, emit_ofs + 0));
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
    const auto op = GetOp29(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit29(index, emit_ofs + 0));
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
    const auto op = GetOp30(index);
    const int consumed = op & 7;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 3;
    sink_(GetEmit30(index, emit_ofs + 0));
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
        const auto op = GetOp31(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit31(index, op >> 2));
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
    const auto op = GetOp32(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit32(index, emit_ofs + 0));
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
        const auto op = GetOp33(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit33(index, op >> 2));
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
    const auto op = GetOp34(index);
    const int consumed = op & 7;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 4;
    switch ((op >> 3) & 1) {
      case 0: {
        sink_(GetEmit34(index, emit_ofs + 0));
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
        const auto op = GetOp35(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit35(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 5: {
        const auto index = buffer_ & 31;
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
}  // namespace geometry_12_12_6
}  // namespace grpc_core
#endif  // GRPC_TEST_CPP_MICROBENCHMARKS_HUFFMAN_GEOMETRIES_DECODE_HUFF_12_12_6_H
