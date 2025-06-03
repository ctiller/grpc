// Copyright 2025 gRPC authors.
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

#include <initializer_list>
#include <limits>
#include <type_traits>

#include "absl/strings/string_view.h"
#include "src/core/util/upb_utils.h"
#include "src/proto/grpc/channelz/v2/cpp.upb.h"
#include "src/proto/grpc/channelz/v2/cpp.upb_minitable.h"

namespace grpc_core {
namespace channelz {

namespace property_list_detail {
template <typename T>
void PropertyValueFromValue(T value, grpc_channelz_v2_PropertyValue* p,
                            upb_Arena* arena) {
  if constexpr (std::is_unsigned_v<T> &&
                std::numeric_limits<T>::max() <=
                    std::numeric_limits<uint64_t>::max()) {
    grpc_channelz_v2_PropertyValue_set_unsigned_int(p, value);
  } else if constexpr (std::is_signed_v<T> &&
                       std::numeric_limits<T>::min() >=
                           std::numeric_limits<int64_t>::min() &&
                       std::numeric_limits<T>::max() <=
                           std::numeric_limits<int64_t>::max()) {
    grpc_channelz_v2_PropertyValue_set_signed_int(p, value);
  } else if constexpr (std::is_floating_point_v<T>) {
    grpc_channelz_v2_PropertyValue_set_number(p, value);
  } else if constexpr (std::is_same_v<absl::string_view, T>) {
    grpc_channelz_v2_PropertyValue_set_string(p, StdStringToUpbString(value));
  } else if constexpr (std::is_same_v<std::string, T>) {
    auto* s = upb_Arena_NewSized(value.size());
    memcpy(s, value.data(), value.size());
    grpc_channelz_v2_PropertyValue_set_string(
        p, upb_StringView_FromDataAndSize(s, value.size()));
  } else if constexpr (std::is_same_v<bool, T>) {
    grpc_channelz_v2_PropertyValue_set_boolean(p, value);
  } else {
    static_assert(false, "Unsupported type");
  }
}
template <typename T>
grpc_channelz_v2_PropertyValue* PropertyValueFromValue(T value,
                                                       upb_Arena* arena) {
  grpc_channelz_v2_PropertyValue* p = grpc_channelz_v2_PropertyValue_new(arena);
  PropertyValueFromValue(std::move(value), p, arena);
  return p;
}
}  // namespace property_list_detail

class PropertyTable {
 public:
  explicit PropertyTable(upb_Arena* arena) : arena_(arena) {}
  PropertyTable(grpc_channelz_v2_PropertyTable* t, upb_Arena* arena)
      : arena_(arena), property_table_(t) {}

  template <typename T>
  void Set(absl::string_view column, absl::string_view row, T value) {
    property_list_detail::PropertyValueFromValue(
        std::move(value), MutableValue(column, row), arena_);
  }

 private:
  grpc_channelz_v2_PropertyValue* MutableValue(absl::string_view column,
                                               absl::string_view row);
  int LookupColumn(absl::string_view column);
  int LookupRow(absl::string_view row);
  grpc_channelz_v2_PropertyValue* MutableValue(int column, int row);

  upb_Arena* arena_;
  absl::flat_hash_map<absl::string_view, int> columns_;
  absl::flat_hash_map<absl::string_view, int> rows_;
  grpc_channelz_v2_PropertyTable* property_table_ =
      grpc_channelz_v2_PropertyTable_new(arena_);
};

class PropertyList;

class PropertyArray {
 public:
  PropertyArray(grpc_channelz_v2_PropertyValue_Array* array, upb_Arena* arena)
      : array_(array), arena_(arena) {}

  template <typename T>
  void Add(T value) {
    property_list_detail::PropertyValueFromValue(
        std::move(value),
        grpc_channelz_v2_PropertyValue_Array_add_values(array_, arena_),
        arena_);
  }

  PropertyList AddPropertyList();

  template <typename T>
  void AddPromise(const T& value) {
    grpc_channelz_v2_Promise* promise = grpc_channelz_v2_Promise_new(arena_);
    PromiseAsProto(value, promise, arena_);
    AddData("type.googleapis.com/grpc.channelz.v2.Promise",
            (upb_Message*)promise, &grpc__channelz__v2__Promise_msg_init);
  }

  void AddData(absl::string_view type_url, const upb_Message* object,
               const upb_MiniTable* object_minitable);

 private:
  grpc_channelz_v2_PropertyValue_Array* array_;
  upb_Arena* arena_;
};

class PropertyList {
 public:
  explicit PropertyList(upb_Arena* arena) : arena_(arena) {}

  template <typename T>
  void Add(absl::string_view key, T value) {
    grpc_channelz_v2_PropertyList_values_set(
        property_list_, StdStringToUpbString(key),
        property_list_detail::PropertyValueFromValue(std::move(value), arena_));
  }

  template <typename T>
  void AddPromise(absl::string_view key, const T& value) {
    grpc_channelz_v2_Promise* promise = grpc_channelz_v2_Promise_new(arena_);
    PromiseAsProto(value, promise, arena_);
    AddData(key, "type.googleapis.com/grpc.channelz.v2.Promise",
            (upb_Message*)promise, &grpc__channelz__v2__Promise_msg_init);
  }

  PropertyArray AddArray(absl::string_view key) {
    grpc_channelz_v2_PropertyValue* value =
        grpc_channelz_v2_PropertyValue_new(arena_);
    auto* array = grpc_channelz_v2_PropertyValue_mutable_array(value, arena_);
    grpc_channelz_v2_PropertyList_values_set(
        property_list_, StdStringToUpbString(key), value, arena_);
    return PropertyArray(array, arena_);
  }

  PropertyList AddPropertyList(absl::string_view key) {
    grpc_channelz_v2_PropertyValue* value =
        grpc_channelz_v2_PropertyValue_new(arena_);
    auto* list =
        grpc_channelz_v2_PropertyValue_mutable_property_list(value, arena_);
    grpc_channelz_v2_PropertyList_values_set(
        property_list_, StdStringToUpbString(key), value, arena_);
    return PropertyList(list, arena_);
  }

  void AddData(absl::string_view key, absl::string_view type_url,
               const upb_Message* object,
               const upb_MiniTable* object_minitable);

  PropertyTable AddTable(absl::string_view key);

 private:
  PropertyList(grpc_channelz_v2_PropertyList* list, upb_Arena* arena)
      : arena_(arena), property_list_(list) {}

  upb_Arena* arena_;
  grpc_channelz_v2_PropertyList* property_list_ =
      grpc_channelz_v2_PropertyList_new(arena_);
};

}  // namespace channelz
}  // namespace grpc_core
