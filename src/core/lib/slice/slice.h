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

#ifndef GRPC_CORE_LIB_SLICE_H
#define GRPC_CORE_LIB_SLICE_H

#include <grpc/impl/codegen/port_platform.h>

#include <string>

#include "src/core/lib/slice/slice_string_helpers.h"
#include "src/core/lib/slice/static_slice.h"

namespace grpc_core {

// Slice represents an owned slice.
// Since copying used to mean an unowned transfer for grpc_slice, and much code
// that used that type will be converted, we disallow copies and instead ask
// that they be made via the Ref() function.
class Slice {
 public:
  Slice() : slice_(GRPC_MDSTR_EMPTY) {}
  // Construct from a grpc_slice -- takes ownership.
  explicit Slice(grpc_slice slice) : slice_(slice) {}
  Slice(const Slice& other) = delete;
  Slice& operator=(const Slice& other) = delete;
  Slice(Slice&& other) noexcept : slice_(other.slice_) {
    other.slice_ = GRPC_MDSTR_EMPTY;
  }
  Slice& operator=(Slice&& other) noexcept {
    std::swap(slice_, other.slice_);
    return *this;
  }
  ~Slice() { grpc_slice_unref_internal(slice_); }

  // Factory function: construct a new slice from a C++ string.
  static Slice FromString(std::string s) {
    return Slice(grpc_slice_from_cpp_string(std::move(s)));
  }

  // Make a new Slice name that owns a reference to the same underlying slice.
  Slice Ref() const { return Slice(grpc_slice_ref_internal(slice_)); }

  // Return a human readable string representation of the slice.
  // Optionally takes some GPR_DUMP_* flags to control the formatting, but
  // defaults to something sensible.
  std::string DebugString(uint32_t format = GPR_DUMP_ASCII |
                                            GPR_DUMP_HEX) const {
    char* s = grpc_dump_slice(slice_, format);
    std::string out(s);
    gpr_free(s);
    return out;
  }

  // Fetch an unowned reference to the underlying slice.
  const grpc_slice& c_slice() const { return slice_; }

 private:
  // Underlying slice object.
  grpc_slice slice_;
};

}  // namespace grpc_core

#endif  // GRPC_CORE_LIB_SLICE_H
