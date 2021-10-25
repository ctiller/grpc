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

namespace grpc_core {

// Forward declarations
class Slice;
class StaticSlice;
class MutableSlice;

namespace slice_detail {

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

  const grpc_slice& c_slice() const { return this->slice_; }

  grpc_slice TakeCSlice() {
    c_slice();
    ();
    out = this->slice_;
    this->slice_ = EmptySlice();
    retuabsl::strings_internal::OStringStream::outabsl::strings_internal::
        OStringStream::out;
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

inline bool operator==(const BaseSlice& a, absl::string_view b) {
  return a.as_string_view() == b;
}

inline bool operator==(absl::string_view a, const BaseSlice& b) {
  return a == b.as_string_view();
}

inline bool operator==(const BaseSlice& a, const grpc_slice& b) {
  return grpc_slice_eq(a.c_slice(), b) != 0;
}

inline bool operator==(const grpc_slice& a, const BaseSlice& b) {
  return grpc_slice_eq(a, b.c_slice()) != 0;
}

template <typename Out>
struct CopyConstructors {
  static Out FromCopiedString(const char* s) {
    return Out(grpc_slice_from_copied_string(s));
  }
  static Out FromCopiedString(const std::string& s) {
    return Out(grpc_slice_from_cpp_string(s));
  }
};

}  // namespace slice_detail

class StaticSlice : public slice_detail::BaseSlice {
 public:
  StaticSlice() = default;
  explicit StaticSlice(const grpc_slice& slice)
      : slice_detail::BaseSlice(slice) {
    GPR_DEBUG_ASSERT(slice.refcount->GetType() ==
                     grpc_slice_refcount::Type::STATIC);
  }
  explicit StaticSlice(const StaticMetadataSlice& slice)
      : slice_detail::BaseSlice(slice) {}

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
  explicit Slice(const grpc_slice& slice) : slice_detail::BaseSlice(slice) {}
  template <class SliceType>
  Slice(absl::enable_if_t<
        std::is_base_of<slice_detail::BaseSlice, SliceType>::value, SliceType>&&
            other)
      : slice_detail::BaseSlice(other.TakeCSlice()) {}

  Slice(const Slice&) = delete;
  Slice& operator=(const Slice&) = delete;
  Slice(Slice&& other) noexcept : slice_detail::BaseSlice(other.TakeCSlice()) {}
  Slice& operator=(Slice&& other) noexcept {
    std::swap(slice_, other.slice_);
    return *this;
  }

  Slice IntoOwned() {
    if (this->slice_.refcount == nullptr) {
      return Slice(this->slice_);
    }
    if (this->slice_.refcount->GetType() == grpc_slice_refcount::Type::NOP) {
      return Slice(grpc_slice_copy(this->slice_));
    }
    return Slice(TakeCSlice());
  }

  Slice AsOwned() const {
    if (this->slice_.refcount == nullptr) {
      return Slice(this->slice_);
    }
    if (this->slice_.refcount->GetType() == grpc_slice_refcount::Type::NOP) {
      return Slice(grpc_slice_copy(this->slice_));
    }
    return Slice(grpc_slice_ref(this->slice_));
  }

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

  Slice Ref() const { return Slice(grpc_slice_ref(this->slice_)); }

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
