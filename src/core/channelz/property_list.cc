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

#include "src/core/channelz/property_list.h"

#include "google/protobuf/any.upb.h"
#include "src/proto/grpc/channelz/v2/cpp.upb.h"

namespace grpc_core::channelz {

void PropertyTable::SetValue(absl::string_view column, absl::string_view row,
                             grpc_channelz_v2_PropertyValue* value) {
  SetIndex(LookupColumn(column), LookupRow(row), value);
}

int PropertyTable::LookupColumn(absl::string_view column) {
  auto it = columns_.find(column);
  if (it != columns_.end()) return it->second;
  const int idx = columns_.size();
  columns_.emplace(column, idx);
  grpc_channelz_v2_PropertyTable_add_columns(
      property_table_, StdStringToUpbString(column), arena_);
  auto* r =
      grpc_channelz_v2_PropertyTable_mutable_rows(property_table_, nullptr);
  for (int row = 0; row < rows_.size(); ++row) {
    grpc_channelz_v2_PropertyTable_Row_add_values(r[row], arena_);
  }
}

int PropertyTable::LookupRow(absl::string_view row) {
  auto it = rows_.find(row);
  if (it != rows_.end()) return it->second;
  const int idx = rows_.size();
  rows_.emplace(row, idx);
  auto* r = grpc_channelz_v2_PropertyTable_add_rows(property_table_, arena_);
  grpc_channelz_v2_PropertyTable_Row_set_label(r, StdStringToUpbString(row));
  grpc_channelz_v2_PropertyTable_Row_resize_values(r, columns_.size(), arena_);
}

void PropertyTable::SetIndex(int column, int row,
                             grpc_channelz_v2_PropertyValue* value) {
  grpc_channelz_v2_PropertyTable_Row_mutable_values(
      grpc_channelz_v2_PropertyTable_mutable_rows(property_table_,
                                                  nullptr)[row],
      nullptr)[column] = value;
}

void PropertyList::AddData(absl::string_view key, absl::string_view type_url,
                           const upb_Message* object,
                           const upb_MiniTable* object_minitable) {
  auto* value = grpc_channelz_v2_PropertyValue_new(arena_);
  google_protobuf_Any* any = google_protobuf_Any_new(arena_);
  google_protobuf_Any_set_type_url(any, StdStringToUpbString(type_url));
  size_t serialized_len;
  char* serialized_bytes;
  if (kUpb_EncodeStatus_Ok != upb_Encode(object, object_minitable, 0, arena_,
                                         &serialized_bytes, &serialized_len)) {
    return;
  }
  google_protobuf_Any_set_value(
      any, upb_StringView_FromDataAndSize(serialized_bytes, serialized_len));
  grpc_channelz_v2_PropertyValue_set_any(value, any);
}

PropertyTable PropertyList::AddTable(absl::string_view key) {
  auto* t = grpc_channelz_v2_PropertyTable_new(arena_);
  AddData(key, "type.googleapis.com/grpc.channelz.v2.PropertyTable",
          (upb_Message*)t, &grpc__channelz__v2__PropertyTable_msg_init);
  return PropertyTable(t, arena_);
}

PropertyList PropertyArray::AddPropertyList() {
  grpc_channelz_v2_PropertyValue* value =
      grpc_channelz_v2_PropertyValue_new(arena_);
  auto* list =
      grpc_channelz_v2_PropertyValue_mutable_property_list(value, arena_);
  grpc_channelz_v2_PropertyList_values_set(
      property_list_, StdStringToUpbString(key), value, arena_);
  return PropertyList(list, arena_);
}
}  // namespace grpc_core::channelz
