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

#ifndef GRPC_CORE_LIB_SLICE_SLICE_H
#define GRPC_CORE_LIB_SLICE_SLICE_H

#include <grpc/support/port_platform.h>

#include "absl/strings/string_view.h"

#include <grpc/slice.h>

#include "src/core/lib/slice/slice_internal.h"

// Herein lies grpc_core::Slice and its team of thin wrappers around grpc_slice.
// They aim to keep you safe by providing strong guarantees around lifetime and
// mutability.
//
// The team:
//   Slice        - provides a wrapper around an unknown type of slice.
//                  Immutable (since we don't know who else might be referencing
//                  it), and potentially ref counted.
//   StaticSlice  - provides a wrapper around a static slice. Not refcounted,
//                  fast to copy.
//   MutableSlice - provides a guarantee of unique ownership, meaning the
//                  underlying data can be mutated safely.

namespace grpc_core {

// Forward declarations
class Slice;
class StaticSlice;
class MutableSlice;

namespace slice_detail {

// Returns an empty slice.
static constexpr grpc_slice EmptySlice() { return {nullptr, {}}; }

// BaseSlice holds the grpc_slice object, but does not apply refcounting policy.
// It does export immutable access into the slice, such that this can be shared
// by all storage policies.
class BaseSlice {
 public:
  BaseSlice(const BaseSlice&) = delete;
  BaseSlice& operator=(const BaseSlice&) = delete;
  BaseSlice(BaseSlice&& other) = delete;
  BaseSlice& operator=(BaseSlice&& other) = delete;

  // Iterator access to the underlying bytes
  const uint8_t* begin() const { return GRPC_SLICE_START_PTR(this->slice_); }
  const uint8_t* end() const { return GRPC_SLICE_END_PTR(this->slice_); }
  const uint8_t* cbegin() const { return GRPC_SLICE_START_PTR(this->slice_); }
  const uint8_t* cend() const { return GRPC_SLICE_END_PTR(this->slice_); }

  // Retrieve a borrowed reference to the underlying grpc_slice.
  const grpc_slice& c_slice() const { return this->slice_; }

  // Retrieve the underlying grpc_slice, and replace the one in this object with
  // EmptySlice().
  grpc_slice TakeCSlice() {
    grpc_slice out = this->slice_;
    this->slice_ = EmptySlice();
    return out;
  }

  // As other things... borrowed references.
  absl::string_view as_string_view() const {
    return absl::string_view(reinterpret_cast<const char*>(data()), size());
  }

  // Array access
  uint8_t operator[](size_t i) const {
    return GRPC_SLICE_START_PTR(this->slice_)[i];
  }

  // Access underlying data
  const uint8_t* data() const { return GRPC_SLICE_START_PTR(this->slice_); }

  // Size of the slice
  size_t size() const { return GRPC_SLICE_LENGTH(this->slice_); }
  size_t length() const { return size(); }
  bool empty() const { return size() == 0; }

  // For inlined slices - are these two slices equal?
  // For non-inlined slices - do these two slices refer to the same block of
  // memory?
  bool is_equivalent(const BaseSlice& other) const {
    return grpc_slice_is_equivalent(slice_, other.slice_);
  }

 protected:
  BaseSlice() : slice_(EmptySlice()) {}
  explicit BaseSlice(const grpc_slice& slice) : slice_(slice) {}
  ~BaseSlice() = default;
  grpc_slice slice_;
};

inline bool operator==(const BaseSlice& a, const BaseSlice& b) {
  return grpc_slice_eq(a.c_slice(), b.c_slice()) != 0;
}

inline bool operator!=(const BaseSlice& a, const BaseSlice& b) {
  return grpc_slice_eq(a.c_slice(), b.c_slice()) == 0;
}

inline bool operator==(const BaseSlice& a, absl::string_view b) {
  return a.as_string_view() == b;
}

inline bool operator!=(const BaseSlice& a, absl::string_view b) {
  return a.as_string_view() != b;
}

inline bool operator==(absl::string_view a, const BaseSlice& b) {
  return a == b.as_string_view();
}

inline bool operator!=(absl::string_view a, const BaseSlice& b) {
  return a != b.as_string_view();
}

inline bool operator==(const BaseSlice& a, const grpc_slice& b) {
  return grpc_slice_eq(a.c_slice(), b) != 0;
}

inline bool operator!=(const BaseSlice& a, const grpc_slice& b) {
  return grpc_slice_eq(a.c_slice(), b) == 0;
}

inline bool operator==(const grpc_slice& a, const BaseSlice& b) {
  return grpc_slice_eq(a, b.c_slice()) != 0;
}

inline bool operator!=(const grpc_slice& a, const BaseSlice& b) {
  return grpc_slice_eq(a, b.c_slice()) == 0;
}

template <typename Out>
struct CopyConstructors {
  static Out FromCopiedString(const char* s) {
    return Out(grpc_slice_from_copied_string(s));
  }
  static Out FromCopiedString(std::string s) {
    return Out(grpc_slice_from_cpp_string(std::move(s)));
  }
};

}  // namespace slice_detail

class StaticSlice : public slice_detail::BaseSlice {
 public:
  StaticSlice() = default;
  explicit StaticSlice(const grpc_slice& slice)
      : slice_detail::BaseSlice(slice) {
    GPR_DEBUG_ASSERT(
        slice.refcount->GetType() == grpc_slice_refcount::Type::STATIC ||
        slice.refcount->GetType() == grpc_slice_refcount::Type::NOP);
  }
  explicit StaticSlice(const StaticMetadataSlice& slice)
      : slice_detail::BaseSlice(slice) {}

  static StaticSlice FromStaticString(const char* s) {
    return StaticSlice(grpc_slice_from_static_string(s));
  }

  StaticSlice(const StaticSlice& other)
      : slice_detail::BaseSlice(other.slice_) {}
  StaticSlice& operator=(const StaticSlice& other) {
    slice_ = other.slice_;
    return *this;
  }
  StaticSlice(StaticSlice&& other) noexcept
      : slice_detail::BaseSlice(other.TakeCSlice()) {}
  StaticSlice& operator=(StaticSlice&& other) noexcept {
    std::swap(slice_, other.slice_);
    return *this;
  }
};

class MutableSlice : public slice_detail::BaseSlice,
                     public slice_detail::CopyConstructors<MutableSlice> {
 public:
  MutableSlice() = default;
  explicit MutableSlice(const grpc_slice& slice)
      : slice_detail::BaseSlice(slice) {
    GPR_DEBUG_ASSERT(slice.refcount == nullptr ||
                     slice.refcount->IsRegularUnique());
  }
  ~MutableSlice() { grpc_slice_unref_internal(slice_); }

  MutableSlice(const MutableSlice&) = delete;
  MutableSlice& operator=(const MutableSlice&) = delete;
  MutableSlice(MutableSlice&& other) noexcept
      : slice_detail::BaseSlice(other.TakeCSlice()) {}
  MutableSlice& operator=(MutableSlice&& other) noexcept {
    std::swap(slice_, other.slice_);
    return *this;
  }

  // Iterator access to the underlying bytes
  uint8_t* begin() { return GRPC_SLICE_START_PTR(this->slice_); }
  uint8_t* end() { return GRPC_SLICE_END_PTR(this->slice_); }

  // Array access
  uint8_t& operator[](size_t i) {
    return GRPC_SLICE_START_PTR(this->slice_)[i];
  }
};

class Slice : public slice_detail::BaseSlice,
              public slice_detail::CopyConstructors<Slice> {
 public:
  Slice() = default;
  ~Slice() { grpc_slice_unref_internal(slice_); }
  explicit Slice(const grpc_slice& slice) : slice_detail::BaseSlice(slice) {}
  template <class SliceType>
  explicit Slice(absl::enable_if_t<
                 std::is_base_of<slice_detail::BaseSlice, SliceType>::value,
                 SliceType>&& other)
      : slice_detail::BaseSlice(other.TakeCSlice()) {}

  Slice(const Slice&) = delete;
  Slice& operator=(const Slice&) = delete;
  Slice(Slice&& other) noexcept : slice_detail::BaseSlice(other.TakeCSlice()) {}
  Slice& operator=(Slice&& other) noexcept {
    std::swap(slice_, other.slice_);
    return *this;
  }

  // A slice might refer to some memory that we keep a refcount to (this is
  // owned), or some memory that's inlined into the slice (also owned), or some
  // other block of memory that we know will be available for the lifetime of
  // some operation in the common case (not owned). In the *less common* case
  // that we need to keep that slice text for longer than our API's guarantee us
  // access, we need to take a copy and turn this into something that we do own.

  // IntoOwned returns an owned slice regardless of current ownership, and
  // leaves the current slice empty - in doing so it can avoid adding a ref to
  // the underlying slice.
  Slice IntoOwned() {
    if (this->slice_.refcount == nullptr) {
      return Slice(this->slice_);
    }
    if (this->slice_.refcount->GetType() == grpc_slice_refcount::Type::NOP) {
      return Slice(grpc_slice_copy(this->slice_));
    }
    return Slice(TakeCSlice());
  }

  // AsOwned returns an owned slice but does not mutate the current slice,
  // meaning that it may add a reference to the underlying slice.
  Slice AsOwned() const {
    if (this->slice_.refcount == nullptr) {
      return Slice(this->slice_);
    }
    if (this->slice_.refcount->GetType() == grpc_slice_refcount::Type::NOP) {
      return Slice(grpc_slice_copy(this->slice_));
    }
    return Slice(grpc_slice_ref_internal(this->slice_));
  }

  // IntoMutable returns a MutableSlice, and leaves the current slice empty.
  // A mutable slice requires only one reference to the bytes of the slice -
  // this can be achieved either with inlined storage or with a single
  // reference.
  // If the current slice is refcounted and there are more than one references
  // to that slice, then the slice is copied in order to achieve a mutable
  // version.
  MutableSlice IntoMutable() {
    if (this->slice_.refcount == nullptr) {
      return MutableSlice(this->slice_);
    }
    if (this->slice_.refcount->GetType() ==
            grpc_slice_refcount::Type::REGULAR &&
        this->slice_.refcount->IsRegularUnique()) {
      return MutableSlice(TakeCSlice());
    }
    return MutableSlice(grpc_slice_copy(this->slice_));
  }

  Slice Ref() const { return Slice(grpc_slice_ref_internal(this->slice_)); }

  static Slice FromRefcountAndBytes(grpc_slice_refcount* r,
                                    const uint8_t* begin, const uint8_t* end) {
    grpc_slice out;
    out.refcount = r;
    r->Ref();
    out.data.refcounted.bytes = const_cast<uint8_t*>(begin);
    out.data.refcounted.length = end - begin;
    return Slice(out);
  }
};

}  // namespace grpc_core

#endif  // GRPC_CORE_LIB_SLICE_SLICE_H
