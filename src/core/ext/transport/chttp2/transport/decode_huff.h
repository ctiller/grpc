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

////////////////////////////////////////////////////////////////////////////////
// This file is autogenerated: see
// tools/codegen/core/gen_huffman_decompressor.cc

#ifndef SRC_CORE_EXT_TRANSPORT_CHTTP2_TRANSPORT_DECODE_HUFF_H
#define SRC_CORE_EXT_TRANSPORT_CHTTP2_TRANSPORT_DECODE_HUFF_H
#include <cstddef>
#include <cstdint>
// GEOMETRY: 8,7,8,5
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
  static inline uint64_t GetOp1(size_t i) {
    return table1_0_inner_[table1_0_outer_[i]];
  }
  static inline uint64_t GetEmit1(size_t, size_t emit) {
    return table1_0_emit_[emit];
  }
  static inline uint64_t GetOp5(size_t i) {
    return table5_0_inner_[table5_0_outer_[i]];
  }
  static inline uint64_t GetEmit5(size_t, size_t emit) {
    return table5_0_emit_[emit];
  }
  static inline uint64_t GetOp7(size_t i) { return table7_0_ops_[i]; }
  static inline uint64_t GetEmit7(size_t, size_t emit) {
    return table7_0_emit_[emit];
  }
  static inline uint64_t GetOp8(size_t i) {
    return table8_0_inner_[table8_0_outer_[i]];
  }
  static inline uint64_t GetEmit8(size_t, size_t emit) {
    return table8_0_emit_[emit];
  }
  static inline uint64_t GetOp9(size_t i) {
    return table9_0_inner_[table9_0_outer_[i]];
  }
  static inline uint64_t GetEmit9(size_t, size_t emit) {
    return table9_0_emit_[emit];
  }
  static inline uint64_t GetOp10(size_t i) { return table10_0_ops_[i]; }
  static inline uint64_t GetEmit10(size_t, size_t emit) {
    return table10_0_emit_[emit];
  }
  static inline uint64_t GetOp11(size_t i) { return table11_0_ops_[i]; }
  static inline uint64_t GetEmit11(size_t, size_t emit) {
    return table11_0_emit_[emit];
  }
  static inline uint64_t GetOp6(size_t i) {
    return table6_ops_[i >> 6][i & 0x3f];
  }
  static inline uint64_t GetEmit6(size_t i, size_t emit) {
    return table6_emit_[i >> 6][emit];
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
  static inline uint64_t GetOp16(size_t i) { return table16_0_ops_[i]; }
  static inline uint64_t GetEmit16(size_t, size_t emit) {
    return table16_0_emit_[emit];
  }
  static inline uint64_t GetOp12(size_t i) {
    return table12_0_inner_[table12_0_outer_[i]];
  }
  static inline uint64_t GetEmit12(size_t, size_t emit) {
    return table12_0_emit_[emit];
  }
  static inline uint64_t GetOp17(size_t i) {
    return table17_0_inner_[table17_0_outer_[i]];
  }
  static inline uint64_t GetEmit17(size_t, size_t emit) {
    return table17_0_emit_[emit];
  }
  static inline uint64_t GetOp18(size_t i) {
    return table17_0_inner_[table17_0_outer_[i]];
  }
  static inline uint64_t GetEmit18(size_t, size_t emit) {
    return table18_0_emit_[emit];
  }
  static inline uint64_t GetOp19(size_t i) {
    return table17_0_inner_[table17_0_outer_[i]];
  }
  static inline uint64_t GetEmit19(size_t, size_t emit) {
    return table19_0_emit_[emit];
  }
  static inline uint64_t GetOp20(size_t i) {
    return table17_0_inner_[table17_0_outer_[i]];
  }
  static inline uint64_t GetEmit20(size_t, size_t emit) {
    return table20_0_emit_[emit];
  }
  static inline uint64_t GetOp21(size_t i) {
    return table17_0_inner_[table17_0_outer_[i]];
  }
  static inline uint64_t GetEmit21(size_t, size_t emit) {
    return table21_0_emit_[emit];
  }
  static inline uint64_t GetOp22(size_t i) {
    return table17_0_inner_[table17_0_outer_[i]];
  }
  static inline uint64_t GetEmit22(size_t, size_t emit) {
    return table22_0_emit_[emit];
  }
  static inline uint64_t GetOp23(size_t i) {
    return table5_0_inner_[table23_0_outer_[i]];
  }
  static inline uint64_t GetEmit23(size_t, size_t emit) {
    return table23_0_emit_[emit];
  }
  static inline uint64_t GetOp24(size_t i) {
    return table24_0_inner_[table24_0_outer_[i]];
  }
  static inline uint64_t GetEmit24(size_t, size_t emit) {
    return table24_0_emit_[emit];
  }
  static inline uint64_t GetOp25(size_t i) {
    return table25_0_inner_[table25_0_outer_[i]];
  }
  static inline uint64_t GetEmit25(size_t, size_t emit) {
    return table25_0_emit_[emit];
  }
  static inline uint64_t GetOp27(size_t i) {
    return table27_0_inner_[table27_0_outer_[i]];
  }
  static inline uint64_t GetEmit27(size_t, size_t emit) {
    return table27_0_emit_[emit];
  }
  static inline uint64_t GetOp26(size_t i) {
    return table26_0_inner_[table26_0_outer_[i]];
  }
  static inline uint64_t GetEmit26(size_t, size_t emit) {
    return table26_0_emit_[emit];
  }
  static inline uint64_t GetOp28(size_t i) {
    return table28_0_inner_[table5_0_outer_[i]];
  }
  static inline uint64_t GetEmit28(size_t, size_t emit) {
    return table28_0_emit_[emit];
  }
  static inline uint64_t GetOp30(size_t i) {
    return table30_0_inner_[table30_0_outer_[i]];
  }
  static inline uint64_t GetEmit30(size_t, size_t emit) {
    return table30_0_emit_[emit];
  }
  static inline uint64_t GetOp29(size_t i) {
    return table29_0_inner_[table29_0_outer_[i]];
  }
  static inline uint64_t GetEmit29(size_t, size_t emit) {
    return table29_0_emit_[emit];
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
  static const uint8_t table1_0_emit_[74];
  static const uint16_t table1_0_inner_[76];
  static const uint8_t table1_0_outer_[256];
  static const uint8_t table5_0_emit_[4];
  static const uint8_t table5_0_inner_[4];
  static const uint8_t table5_0_outer_[4];
  static const uint8_t table7_0_emit_[1];
  static const uint8_t table7_0_ops_[4];
  static const uint8_t table8_0_emit_[4];
  static const uint8_t table8_0_inner_[6];
  static const uint8_t table8_0_outer_[8];
  static const uint8_t table9_0_emit_[6];
  static const uint8_t table9_0_inner_[8];
  static const uint8_t table9_0_outer_[16];
  static const uint8_t table10_0_emit_[12];
  static const uint8_t table10_0_ops_[32];
  static const uint8_t table11_0_emit_[14];
  static const uint8_t table11_0_ops_[64];
  static const uint8_t table6_0_emit_[3];
  static const uint8_t table6_0_ops_[64];
  static const uint8_t table6_1_emit_[14];
  static const uint8_t table6_1_ops_[64];
  static const uint8_t* const table6_emit_[2];
  static const uint8_t* const table6_ops_[2];
  static const uint8_t table13_0_emit_[3];
  static const uint8_t table13_0_ops_[16];
  static const uint8_t table14_0_emit_[11];
  static const uint8_t table14_0_ops_[32];
  static const uint8_t table15_0_emit_[24];
  static const uint8_t table15_0_ops_[64];
  static const uint8_t table16_0_emit_[50];
  static const uint8_t table16_0_ops_[128];
  static const uint8_t table12_0_emit_[79];
  static const uint16_t table12_0_inner_[90];
  static const uint8_t table12_0_outer_[256];
  static const uint8_t table17_0_emit_[2];
  static const uint8_t table17_0_inner_[2];
  static const uint8_t table17_0_outer_[2];
  static const uint8_t table18_0_emit_[2];
  static const uint8_t table19_0_emit_[2];
  static const uint8_t table20_0_emit_[2];
  static const uint8_t table21_0_emit_[2];
  static const uint8_t table22_0_emit_[2];
  static const uint8_t table23_0_emit_[4];
  static const uint8_t table23_0_outer_[4];
  static const uint8_t table24_0_emit_[8];
  static const uint8_t table24_0_inner_[8];
  static const uint8_t table24_0_outer_[8];
  static const uint8_t table25_0_emit_[16];
  static const uint8_t table25_0_inner_[16];
  static const uint8_t table25_0_outer_[16];
  static const uint8_t table27_0_emit_[1];
  static const uint8_t table27_0_inner_[3];
  static const uint8_t table27_0_outer_[16];
  static const uint8_t table26_0_emit_[30];
  static const uint16_t table26_0_inner_[31];
  static const uint8_t table26_0_outer_[32];
  static const uint8_t table28_0_emit_[3];
  static const uint8_t table28_0_inner_[4];
  static const uint8_t table30_0_emit_[7];
  static const uint8_t table30_0_inner_[8];
  static const uint8_t table30_0_outer_[8];
  static const uint8_t table29_0_emit_[9];
  static const uint8_t table29_0_inner_[9];
  static const uint8_t table29_0_outer_[16];
};
template <typename F>
class HuffDecoder : public HuffDecoderCommon {
 public:
  HuffDecoder(F sink, const uint8_t* begin, const uint8_t* end)
      : sink_(sink), begin_(begin), end_(end) {}
  bool Run() {
    while (!done_) {
      if (!RefillTo8()) {
        Done0();
        break;
      }
      const auto index = (buffer_ >> (buffer_len_ - 8)) & 0xff;
      const auto op = GetOp1(index);
      const int consumed = op & 15;
      buffer_len_ -= consumed;
      const auto emit_ofs = op >> 6;
      switch ((op >> 4) & 3) {
        case 1: {
          DecodeStep0();
          break;
        }
        case 2: {
          DecodeStep1();
          break;
        }
        case 0: {
          sink_(GetEmit1(index, emit_ofs + 0));
          break;
        }
      }
    }
    return ok_;
  }

 private:
  bool RefillTo8() {
    switch (buffer_len_) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7: {
        return Read1();
      }
    }
    return true;
  }
  bool Read1() {
    if (begin_ + 1 > end_) return false;
    buffer_ <<= 8;
    buffer_ |= static_cast<uint64_t>(*begin_++) << 0;
    buffer_len_ += 8;
    return true;
  }
  void Done0() {
    done_ = true;
    switch (buffer_len_) {
      case 0: {
        break;
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
      case 1:
      case 2:
      case 3:
      case 4: {
        ok_ = (buffer_ & ((1 << buffer_len_) - 1)) == (1 << buffer_len_) - 1;
        return;
      }
    }
  }
  void DecodeStep0() {
    if (!RefillTo2()) {
      // some=0/2;n=33 sym_len=10 sym_bits=1016 consumed_len=8 consumed_mask=254
      // all_ones_so_far=0
      Done1();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 2)) & 0x3;
    const auto op = GetOp5(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit5(index, emit_ofs + 0));
  }
  bool RefillTo2() {
    switch (buffer_len_) {
      case 0:
      case 1: {
        return Read1();
      }
    }
    return true;
  }
  void Done1() {
    done_ = true;
    switch (buffer_len_) {
      case 0: {
        ok_ = false;
        break;
      }
      case 1: {
        ok_ = false;
        return;
      }
    }
  }
  void DecodeStep1() {
    if (!RefillTo7()) {
      // some=18/5;n=0 sym_len=13 sym_bits=8184 consumed_len=8 consumed_mask=255
      // all_ones_so_far=1
      Done2();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 7)) & 0x7f;
    const auto op = GetOp6(index);
    const int consumed = op & 7;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 4;
    switch ((op >> 3) & 1) {
      case 1: {
        DecodeStep2();
        break;
      }
      case 0: {
        sink_(GetEmit6(index, emit_ofs + 0));
        break;
      }
    }
  }
  bool RefillTo7() {
    switch (buffer_len_) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6: {
        return Read1();
      }
    }
    return true;
  }
  void Done2() {
    done_ = true;
    switch (buffer_len_) {
      case 0: {
        break;
      }
      case 4: {
        const auto index = buffer_ & 15;
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
      case 5: {
        const auto index = buffer_ & 31;
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
      case 2: {
        const auto index = buffer_ & 3;
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
      case 6: {
        const auto index = buffer_ & 63;
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
      case 1: {
        ok_ = (buffer_ & ((1 << buffer_len_) - 1)) == (1 << buffer_len_) - 1;
        return;
      }
    }
  }
  void DecodeStep2() {
    if (!RefillTo8()) {
      // some=d8/8;n=1 sym_len=23 sym_bits=8388568 consumed_len=15
      // consumed_mask=32767 all_ones_so_far=1
      Done3();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 8)) & 0xff;
    const auto op = GetOp12(index);
    const int consumed = op & 15;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 8;
    switch ((op >> 4) & 15) {
      case 8: {
        DecodeStep10();
        break;
      }
      case 10: {
        DecodeStep11();
        break;
      }
      case 11: {
        DecodeStep12();
        break;
      }
      case 9: {
        DecodeStep14();
        break;
      }
      case 1: {
        DecodeStep3();
        break;
      }
      case 2: {
        DecodeStep4();
        break;
      }
      case 3: {
        DecodeStep5();
        break;
      }
      case 4: {
        DecodeStep6();
        break;
      }
      case 5: {
        DecodeStep7();
        break;
      }
      case 6: {
        DecodeStep8();
        break;
      }
      case 7: {
        DecodeStep9();
        break;
      }
      case 0: {
        sink_(GetEmit12(index, emit_ofs + 0));
        break;
      }
    }
  }
  void Done3() {
    done_ = true;
    switch (buffer_len_) {
      case 0: {
        break;
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
      case 1:
      case 2:
      case 3: {
        ok_ = (buffer_ & ((1 << buffer_len_) - 1)) == (1 << buffer_len_) - 1;
        return;
      }
    }
  }
  void DecodeStep3() {
    if (!RefillTo1()) {
      // some=0/1;n=9 sym_len=24 sym_bits=16777194 consumed_len=23
      // consumed_mask=8388597 all_ones_so_far=0
      Done4();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 1)) & 0x1;
    const auto op = GetOp17(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit17(index, emit_ofs + 0));
  }
  bool RefillTo1() {
    switch (buffer_len_) {
      case 0: {
        return Read1();
      }
    }
    return true;
  }
  void Done4() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep4() {
    if (!RefillTo1()) {
      // some=0/1;n=144 sym_len=24 sym_bits=16777196 consumed_len=23
      // consumed_mask=8388598 all_ones_so_far=0
      Done5();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 1)) & 0x1;
    const auto op = GetOp18(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit18(index, emit_ofs + 0));
  }
  void Done5() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep5() {
    if (!RefillTo1()) {
      // some=0/1;n=148 sym_len=24 sym_bits=16777198 consumed_len=23
      // consumed_mask=8388599 all_ones_so_far=0
      Done6();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 1)) & 0x1;
    const auto op = GetOp19(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit19(index, emit_ofs + 0));
  }
  void Done6() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep6() {
    if (!RefillTo1()) {
      // some=0/1;n=171 sym_len=24 sym_bits=16777200 consumed_len=23
      // consumed_mask=8388600 all_ones_so_far=0
      Done7();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 1)) & 0x1;
    const auto op = GetOp20(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit20(index, emit_ofs + 0));
  }
  void Done7() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep7() {
    if (!RefillTo1()) {
      // some=0/1;n=215 sym_len=24 sym_bits=16777202 consumed_len=23
      // consumed_mask=8388601 all_ones_so_far=0
      Done8();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 1)) & 0x1;
    const auto op = GetOp21(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit21(index, emit_ofs + 0));
  }
  void Done8() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep8() {
    if (!RefillTo1()) {
      // some=0/1;n=236 sym_len=24 sym_bits=16777204 consumed_len=23
      // consumed_mask=8388602 all_ones_so_far=0
      Done9();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 1)) & 0x1;
    const auto op = GetOp22(index);
    const int consumed = op & 1;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 1;
    sink_(GetEmit22(index, emit_ofs + 0));
  }
  void Done9() {
    done_ = true;
    ok_ = false;
  }
  void DecodeStep9() {
    if (!RefillTo2()) {
      // some=0/2;n=199 sym_len=25 sym_bits=33554412 consumed_len=23
      // consumed_mask=8388603 all_ones_so_far=0
      Done10();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 2)) & 0x3;
    const auto op = GetOp23(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit23(index, emit_ofs + 0));
  }
  void Done10() {
    done_ = true;
    switch (buffer_len_) {
      case 0: {
        ok_ = false;
        break;
      }
      case 1: {
        ok_ = false;
        return;
      }
    }
  }
  void DecodeStep10() {
    if (!RefillTo3()) {
      // some=0/3;n=192 sym_len=26 sym_bits=67108832 consumed_len=23
      // consumed_mask=8388604 all_ones_so_far=0
      Done11();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 3)) & 0x7;
    const auto op = GetOp24(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 2;
    sink_(GetEmit24(index, emit_ofs + 0));
  }
  bool RefillTo3() {
    switch (buffer_len_) {
      case 0:
      case 1:
      case 2: {
        return Read1();
      }
    }
    return true;
  }
  void Done11() {
    done_ = true;
    switch (buffer_len_) {
      case 0: {
        ok_ = false;
        break;
      }
      case 1:
      case 2: {
        ok_ = false;
        return;
      }
    }
  }
  void DecodeStep11() {
    if (!RefillTo4()) {
      // some=0/4;n=211 sym_len=27 sym_bits=134217696 consumed_len=23
      // consumed_mask=8388606 all_ones_so_far=0
      Done12();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 4)) & 0xf;
    const auto op = GetOp25(index);
    const int consumed = op & 7;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 3;
    sink_(GetEmit25(index, emit_ofs + 0));
  }
  bool RefillTo4() {
    switch (buffer_len_) {
      case 0:
      case 1:
      case 2:
      case 3: {
        return Read1();
      }
    }
    return true;
  }
  void Done12() {
    done_ = true;
    switch (buffer_len_) {
      case 0: {
        ok_ = false;
        break;
      }
      case 1:
      case 2:
      case 3: {
        ok_ = false;
        return;
      }
    }
  }
  void DecodeStep12() {
    if (!RefillTo5()) {
      // some=2/5;n=2 sym_len=28 sym_bits=268435426 consumed_len=23
      // consumed_mask=8388607 all_ones_so_far=1
      Done13();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 5)) & 0x1f;
    const auto op = GetOp26(index);
    const int consumed = op & 7;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 4;
    switch ((op >> 3) & 1) {
      case 1: {
        DecodeStep13();
        break;
      }
      case 0: {
        sink_(GetEmit26(index, emit_ofs + 0));
        break;
      }
    }
  }
  bool RefillTo5() {
    switch (buffer_len_) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4: {
        return Read1();
      }
    }
    return true;
  }
  void Done13() {
    done_ = true;
    switch (buffer_len_) {
      case 0: {
        break;
      }
      case 4: {
        const auto index = buffer_ & 15;
        const auto op = GetOp27(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit27(index, op >> 2));
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
    }
  }
  void DecodeStep13() {
    if (!RefillTo2()) {
      // some=0/2;n=10 sym_len=30 sym_bits=1073741820 consumed_len=28
      // consumed_mask=268435455 all_ones_so_far=1
      Done14();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 2)) & 0x3;
    const auto op = GetOp28(index);
    const int consumed = op & 3;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 3;
    switch ((op >> 2) & 1) {
      case 1: {
        begin_ = end_;
        buffer_len_ = 0;
        break;
      }
      case 0: {
        sink_(GetEmit28(index, emit_ofs + 0));
        break;
      }
    }
  }
  void Done14() {
    done_ = true;
    switch (buffer_len_) {
      case 0: {
        break;
      }
      case 1: {
        ok_ = (buffer_ & ((1 << buffer_len_) - 1)) == (1 << buffer_len_) - 1;
        return;
      }
    }
  }
  void DecodeStep14() {
    if (!RefillTo4()) {
      // some=e/4;n=203 sym_len=27 sym_bits=134217694 consumed_len=23
      // consumed_mask=8388605 all_ones_so_far=0
      Done15();
      return;
    }
    const auto index = (buffer_ >> (buffer_len_ - 4)) & 0xf;
    const auto op = GetOp29(index);
    const int consumed = op & 7;
    buffer_len_ -= consumed;
    const auto emit_ofs = op >> 3;
    sink_(GetEmit29(index, emit_ofs + 0));
  }
  void Done15() {
    done_ = true;
    switch (buffer_len_) {
      case 3: {
        const auto index = buffer_ & 7;
        const auto op = GetOp30(index);
        switch (op & 3) {
          case 1: {
            ok_ = false;
            break;
          }
          case 2: {
            sink_(GetEmit30(index, op >> 2));
            break;
          }
        }
        return;
      }
      case 0: {
        ok_ = false;
        break;
      }
      case 1:
      case 2: {
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
#endif