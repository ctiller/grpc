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

#ifndef GRPC_CORE_LIB_GPRPP_BITSET_H
#define GRPC_CORE_LIB_GPRPP_BITSET_H

#include <grpc/impl/codegen/port_platform.h>

#include <utility>

namespace grpc_core {

// Given a bit count as an integer, vend as member type `Type` a type with
// exactly that number of bits. Undefined if that bit count is not available.
template <std::size_t kBits>
struct UintSelector;
template <>
struct UintSelector<8> {
  typedef uint8_t Type;
};
template <>
struct UintSelector<16> {
  typedef uint16_t Type;
};
template <>
struct UintSelector<32> {
  typedef uint32_t Type;
};
template <>
struct UintSelector<64> {
  typedef uint64_t Type;
};

// An unsigned integer of some number of bits.
template <std::size_t kBits>
using Uint = typename UintSelector<kBits>::Type;

// Given the total number of bits that need to be stored, choose the size of
// 'unit' for a BitSet... We'll use an array of units to store the total set.
// For small bit counts we are selective in the type to try and balance byte
// size and performance
// - the details will likely be tweaked into the future.
// Once we get over 96 bits, we just use uint64_t for everything.
constexpr std::size_t ChooseUnitBitsForBitSet(std::size_t total_bits) {
  return total_bits <= 8    ? 8
         : total_bits <= 16 ? 16
         : total_bits <= 24 ? 8
         : total_bits <= 32 ? 32
         : total_bits <= 48 ? 16
         : total_bits <= 64 ? 64
         : total_bits <= 96 ? 32
                            : 64;
}

// A BitSet that's configurable.
// Contains storage for kTotalBits, stored as an array of integers of size
// kUnitBits. e.g. to store 72 bits in 8 bit chunks, we'd say BitSet<72, 8>.
// Since most users shouldn't care about the size of unit used, we default
// kUnitBits to whatever is selected by ChooseUnitBitsForBitSet
template <std::size_t kTotalBits,
          std::size_t kUnitBits = ChooseUnitBitsForBitSet(kTotalBits)>
class BitSet {
  static constexpr std::size_t kUnits =
      (kTotalBits + kUnitBits - 1) / kUnitBits;

 public:
  // Initialize to all bits false
  constexpr BitSet() : units_{} {}

  // Set bit i to true
  void set(int i) { units_[unit_for(i)] |= mask_for(i); }

  // Set bit i to is_set
  void set(int i, bool is_set) {
    if (is_set)
      set(i);
    else
      clear(i);
  }

  // Set bit i to false
  void clear(int i) { units_[unit_for(i)] &= ~mask_for(i); }

  // Return true if bit i is set
  constexpr bool is_set(int i) const {
    return (units_[unit_for(i)] & mask_for(i)) != 0;
  }

  // Return true if all bits are set
  bool all() const {
    if (kTotalBits % kUnitBits == 0) {
      // kTotalBits is a multiple of kUnitBits ==> we can just check for all
      // ones in each unit.
      for (size_t i = 0; i < kUnits; i++) {
        if (units_[i] != ~Uint<kUnitBits>(0)) return false;
      }
    } else {
      // kTotalBits is not a multiple of kUnitBits ==> we need special handling
      // for checking partial filling of the last unit (since not all of its
      // bits are used!)
      for (size_t i = 0; i < kUnits - 1; i++) {
        if (units_[i] != ~Uint<kUnitBits>(0)) return false;
      }
      return units_[kUnits - 1] ==
             (Uint<kUnitBits>(1) << (kTotalBits % kUnitBits)) - 1;
    }
  }

  // Return true if *no* bits are set.
  bool none() const {
    for (size_t i = 0; i < kUnits; i++) {
      if (units_[i] != 0) return false;
    }
    return true;
  }

 private:
  // Given a bit index, return which unit it's stored in.
  static constexpr std::size_t unit_for(std::size_t bit) {
    return bit / kUnitBits;
  }

  // Given a bit index, return a mask to access that bit within it's unit.
  static constexpr Uint<kUnitBits> mask_for(std::size_t bit) {
    return Uint<kUnitBits>{1} << (bit % kUnitBits);
  }

  // The set of units - kUnitBits sized integers that store kUnitBits bits!
  Uint<kUnitBits> units_[kUnits];
};

}  // namespace grpc_core

#endif  // GRPC_CORE_LIB_GPRPP_BITSET_H
