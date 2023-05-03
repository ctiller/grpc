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

#include <grpc/support/port_platform.h>

#include "src/core/ext/transport/chaotic_good/frame_header.h"

#include <string.h>

#include <cstdint>

#include "absl/status/status.h"

namespace grpc_core {
namespace chaotic_good {

namespace {
void WriteLittleEndianUint32(uint32_t value, uint8_t* data) {
  data[0] = static_cast<uint8_t>(value);
  data[1] = static_cast<uint8_t>(value >> 8);
  data[2] = static_cast<uint8_t>(value >> 16);
  data[3] = static_cast<uint8_t>(value >> 24);
}

uint32_t ReadLittleEndianUint32(const uint8_t* data) {
  return static_cast<uint32_t>(data[0]) |
         (static_cast<uint32_t>(data[1]) << 8) |
         (static_cast<uint32_t>(data[2]) << 16) |
         (static_cast<uint32_t>(data[3]) << 24);
}
}  // namespace

void FrameHeader::Serialize(uint8_t* data) const {
  // Serializes a frame header into a buffer of 24 bytes.
  WriteLittleEndianUint32(type_and_flags, data);
  WriteLittleEndianUint32(stream_id, data + 4);
  WriteLittleEndianUint32(header_length, data + 8);
  WriteLittleEndianUint32(message_length, data + 12);
  WriteLittleEndianUint32(message_length, data + 16);
  WriteLittleEndianUint32(trailer_length, data + 20);
}

absl::StatusOr<FrameHeader> FrameHeader::Parse(const uint8_t* data) {
  // Parses a frame header from a buffer of 24 bytes. All 24 bytes are consumed.
  FrameHeader header;
  header.type_and_flags = ReadLittleEndianUint32(data);
  header.stream_id = ReadLittleEndianUint32(data + 4);
  header.header_length = ReadLittleEndianUint32(data + 8);
  header.message_length = ReadLittleEndianUint32(data + 12);
  header.message_padding = ReadLittleEndianUint32(data + 16);
  header.trailer_length = ReadLittleEndianUint32(data + 20);
  return header;
}

namespace {
uint64_t RoundUp(uint64_t x) {
  if (x % 64 == 0) return x;
  return x + 64 - (x % 64);
}
}  // namespace

FrameSizes FrameHeader::ComputeFrameSizes() const {
  FrameSizes sizes;
  sizes.message_offset = RoundUp(header_length);
  sizes.trailer_offset =
      sizes.message_offset + RoundUp(message_length + message_padding);
  sizes.frame_length = sizes.trailer_offset + RoundUp(trailer_length);
  return sizes;
}

}  // namespace chaotic_good
}  // namespace grpc_core
