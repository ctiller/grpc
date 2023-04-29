//
//
// Copyright 2015 gRPC authors.
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
//
//

#include <grpc/support/port_platform.h>

#include "src/core/ext/transport/chttp2/transport/hpack_parser.h"

#include <stddef.h>
#include <stdlib.h>

#include <algorithm>
#include <initializer_list>
#include <string>
#include <utility>

#include "absl/base/attributes.h"
#include "absl/status/status.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "absl/types/variant.h"
#include "hpack_parser_table.h"

#include <grpc/slice.h>
#include <grpc/support/log.h>

#include "src/core/ext/transport/chttp2/transport/decode_huff.h"
#include "src/core/ext/transport/chttp2/transport/hpack_constants.h"
#include "src/core/lib/debug/stats.h"
#include "src/core/lib/debug/stats_data.h"
#include "src/core/lib/debug/trace.h"
#include "src/core/lib/gprpp/crash.h"
#include "src/core/lib/gprpp/match.h"
#include "src/core/lib/gprpp/status_helper.h"
#include "src/core/lib/slice/slice.h"
#include "src/core/lib/slice/slice_refcount.h"
#include "src/core/lib/surface/validate_metadata.h"
#include "src/core/lib/transport/parsed_metadata.h"

// IWYU pragma: no_include <type_traits>

namespace grpc_core {

TraceFlag grpc_trace_chttp2_hpack_parser(false, "chttp2_hpack_parser");

namespace {
// The alphabet used for base64 encoding binary metadata.
constexpr char kBase64Alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

// An inverted table: for each value in kBase64Alphabet, table contains the
// index with which it's stored, so we can quickly invert the encoding without
// any complicated runtime logic.
struct Base64InverseTable {
  uint8_t table[256]{};
  constexpr Base64InverseTable() {
    for (int i = 0; i < 256; i++) {
      table[i] = 255;
    }
    for (const char* p = kBase64Alphabet; *p; p++) {
      uint8_t idx = *p;
      uint8_t ofs = p - kBase64Alphabet;
      table[idx] = ofs;
    }
  }
};

constexpr Base64InverseTable kBase64InverseTable;

absl::Status EnsureStreamError(absl::Status error) {
  if (error.ok()) return error;
  return grpc_error_set_int(std::move(error), StatusIntProperty::kStreamId, 0);
}

bool IsStreamError(const absl::Status& status) {
  intptr_t stream_id;
  return grpc_error_get_int(status, grpc_core::StatusIntProperty::kStreamId,
                            &stream_id);
}

class MetadataSizeLimitExceededEncoder {
 public:
  explicit MetadataSizeLimitExceededEncoder(std::string& summary)
      : summary_(summary) {}

  void Encode(const Slice& key, const Slice& value) {
    AddToSummary(key.as_string_view(), value.size());
  }

  template <typename Key, typename Value>
  void Encode(Key, const Value& value) {
    AddToSummary(Key::key(), EncodedSizeOfKey(Key(), value));
  }

 private:
  void AddToSummary(absl::string_view key,
                    size_t value_length) GPR_ATTRIBUTE_NOINLINE {
    absl::StrAppend(&summary_, " ", key, ":",
                    hpack_constants::SizeForEntry(key.size(), value_length),
                    "B");
  }
  std::string& summary_;
};
}  // namespace

// Input tracks the current byte through the input data and provides it
// via a simple stream interface.
class HPackParser::Input {
 public:
  Input(grpc_slice_refcount* current_slice_refcount, const uint8_t* begin,
        const uint8_t* end)
      : current_slice_refcount_(current_slice_refcount),
        begin_(begin),
        end_(end),
        frontier_(begin) {}

  // If input is backed by a slice, retrieve its refcount. If not, return
  // nullptr.
  grpc_slice_refcount* slice_refcount() { return current_slice_refcount_; }

  // Have we reached the end of input?
  bool end_of_stream() const { return begin_ == end_; }
  // How many bytes until end of input
  size_t remaining() const { return end_ - begin_; }
  // Current position, as a pointer
  const uint8_t* cur_ptr() const { return begin_; }
  // End position, as a pointer
  const uint8_t* end_ptr() const { return end_; }
  // Move read position forward by n, unchecked
  void Advance(size_t n) { begin_ += n; }

  // Retrieve the current character, or nullopt if end of stream
  // Do not advance
  absl::optional<uint8_t> peek() const {
    if (end_of_stream()) {
      return {};
    }
    return *begin_;
  }

  // Retrieve and advance past the current character, or return nullopt if end
  // of stream
  absl::optional<uint8_t> Next() {
    if (end_of_stream()) {
      UnexpectedEOF(1);
      return absl::optional<uint8_t>();
    }
    return *begin_++;
  }

  // Helper to parse a varint delta on top of value, return nullopt on failure
  // (setting error)
  absl::optional<uint32_t> ParseVarint(uint32_t value) {
    // TODO(ctiller): break out a variant of this when we know there are at
    // least 5 bytes in input_
    auto cur = Next();
    if (!cur) return {};
    value += *cur & 0x7f;
    if ((*cur & 0x80) == 0) return value;

    cur = Next();
    if (!cur) return {};
    value += (*cur & 0x7f) << 7;
    if ((*cur & 0x80) == 0) return value;

    cur = Next();
    if (!cur) return {};
    value += (*cur & 0x7f) << 14;
    if ((*cur & 0x80) == 0) return value;

    cur = Next();
    if (!cur) return {};
    value += (*cur & 0x7f) << 21;
    if ((*cur & 0x80) == 0) return value;

    cur = Next();
    if (!cur) return {};
    uint32_t c = (*cur) & 0x7f;
    // We might overflow here, so we need to be a little careful about the
    // addition
    if (c > 0xf) return ParseVarintOutOfRange(value, *cur);
    const uint32_t add = c << 28;
    if (add > 0xffffffffu - value) {
      return ParseVarintOutOfRange(value, *cur);
    }
    value += add;
    if ((*cur & 0x80) == 0) return value;

    // Spec weirdness: we can add an infinite stream of 0x80 at the end of a
    // varint and still end up with a correctly encoded varint.
    // We allow up to 16 just for kicks, but any more and we'll assume the
    // sender is being malicious.
    int num_redundent_0x80 = 0;
    do {
      cur = Next();
      if (!cur.has_value()) return {};
      ++num_redundent_0x80;
      if (num_redundent_0x80 == 16) {
        return ParseVarintMaliciousEncoding();
      }
    } while (*cur == 0x80);

    // BUT... the last byte needs to be 0x00 or we'll overflow dramatically!
    if (*cur == 0) return value;
    return ParseVarintOutOfRange(value, *cur);
  }

  // Parse a string prefix
  absl::optional<StringPrefix> ParseStringPrefix() {
    auto cur = Next();
    if (!cur.has_value()) {
      GPR_DEBUG_ASSERT(eof_error());
      return {};
    }
    // Huffman if the top bit is 1
    const bool huff = (*cur & 0x80) != 0;
    // String length
    uint32_t strlen = (*cur & 0x7f);
    if (strlen == 0x7f) {
      // all ones ==> varint string length
      auto v = ParseVarint(0x7f);
      if (!v.has_value()) {
        GPR_DEBUG_ASSERT(eof_error());
        return {};
      }
      strlen = *v;
    }
    return StringPrefix{strlen, huff};
  }

  // Check if we saw an EOF.. must be verified before looking at TakeError
  bool eof_error() const {
    return min_progress_size_ != 0 || (!error_.ok() && !IsStreamError(error_));
  }

  // Minimum number of bytes to unstuck the current parse
  size_t min_progress_size() const { return min_progress_size_; }

  // Extract the parse error, leaving the current error as NONE.
  grpc_error_handle TakeError() {
    grpc_error_handle out = error_;
    error_ = absl::OkStatus();
    return out;
  }

  bool has_error() const { return !error_.ok(); }

  // Set the current error - tweaks the error to include a stream id so that
  // chttp2 does not close the connection.
  // Intended for errors that are specific to a stream and recoverable.
  // Callers should ensure that any hpack table updates happen.
  GPR_ATTRIBUTE_NOINLINE void SetErrorAndContinueParsing(
      grpc_error_handle error) {
    GPR_ASSERT(!error.ok());
    // StreamId is used as a signal to skip this stream but keep the connection
    // alive
    SetError(EnsureStreamError(std::move(error)));
  }

  // Set the current error, and skip past remaining bytes.
  // Intended for unrecoverable errors, with the expectation that they will
  // close the connection on return to chttp2.
  GPR_ATTRIBUTE_NOINLINE void SetErrorAndStopParsing(grpc_error_handle error) {
    GPR_ASSERT(!error.ok());
    SetError(std::move(error));
    begin_ = end_;
  }

  // Set the error to an unexpected eof
  void UnexpectedEOF(size_t min_progress_size) {
    GPR_ASSERT(min_progress_size > 0);
    if (min_progress_size_ != 0 || (!error_.ok() && !IsStreamError(error_))) {
      return;
    }
    // Set min progress size, taking into account bytes parsed already but not
    // consumed.
    min_progress_size_ = min_progress_size + (begin_ - frontier_);
  }

  // Update the frontier - signifies we've successfully parsed another element
  void UpdateFrontier() {
    GPR_DEBUG_ASSERT(skip_bytes_ == 0);
    frontier_ = begin_;
  }

  void UpdateFrontierAndSkipBytes(size_t skip_bytes) {
    UpdateFrontier();
    size_t remaining = end_ - begin_;
    if (skip_bytes >= remaining) {
      skip_bytes_ = skip_bytes - remaining;
      frontier_ = end_;
    } else {
      frontier_ += skip_bytes_;
    }
  }

  // Get the frontier - for buffering should we fail due to eof
  const uint8_t* frontier() const { return frontier_; }

 private:
  // Helper to set the error to out of range for ParseVarint
  absl::optional<uint32_t> ParseVarintOutOfRange(uint32_t value,
                                                 uint8_t last_byte) {
    SetErrorAndStopParsing(absl::InternalError(absl::StrFormat(
        "integer overflow in hpack integer decoding: have 0x%08x, "
        "got byte 0x%02x on byte 5",
        value, last_byte)));
    return absl::optional<uint32_t>();
  }

  // Helper to set the error in the case of a malicious encoding
  absl::optional<uint32_t> ParseVarintMaliciousEncoding() {
    SetErrorAndStopParsing(
        absl::InternalError("Malicious varint encoding observed during hpack "
                            "decoding (infinite length degenerate varint)"));
    return absl::optional<uint32_t>();
  }

  // If no error is set, set it to the given error (i.e. first error wins)
  // Do not use this directly, instead use SetErrorAndContinueParsing or
  // SetErrorAndStopParsing.
  void SetError(grpc_error_handle error) {
    if (!error_.ok() || min_progress_size_ != 0) {
      if (!IsStreamError(error) && IsStreamError(error_)) {
        error_ = std::move(error);  // connection errors dominate
      }
      return;
    }
    error_ = std::move(error);
  }

  // Refcount if we are backed by a slice
  grpc_slice_refcount* current_slice_refcount_;
  // Current input point
  const uint8_t* begin_;
  // End of stream point
  const uint8_t* const end_;
  // Frontier denotes the first byte past successfully processed input
  const uint8_t* frontier_;
  // Current error
  grpc_error_handle error_;
  // If the error was EOF, we flag it here by noting how many more bytes would
  // be needed to make progress
  size_t min_progress_size_ = 0;
  // Number of bytes that should be skipped before parsing resumes
  size_t skip_bytes_ = 0;
};

absl::string_view HPackParser::String::string_view() const {
  if (auto* p = absl::get_if<Slice>(&value_)) {
    return p->as_string_view();
  } else if (auto* p = absl::get_if<absl::Span<const uint8_t>>(&value_)) {
    return absl::string_view(reinterpret_cast<const char*>(p->data()),
                             p->size());
  } else if (auto* p = absl::get_if<std::vector<uint8_t>>(&value_)) {
    return absl::string_view(reinterpret_cast<const char*>(p->data()),
                             p->size());
  }
  GPR_UNREACHABLE_CODE(return absl::string_view());
}

template <typename Out>
HPackParser::String::ParseStatus HPackParser::String::ParseHuff(Input* input,
                                                                uint32_t length,
                                                                Out output) {
  // If there's insufficient bytes remaining, return now.
  if (input->remaining() < length) {
    input->UnexpectedEOF(length);
    GPR_DEBUG_ASSERT(input->eof_error());
    return ParseStatus::kEof;
  }
  // Grab the byte range, and iterate through it.
  const uint8_t* p = input->cur_ptr();
  input->Advance(length);
  return HuffDecoder<Out>(output, p, p + length).Run()
             ? ParseStatus::kOk
             : ParseStatus::kParseHuffFailed;
}

struct HPackParser::String::ParseResult {
  ParseResult() = delete;
  ParseResult(ParseStatus status, size_t wire_size, String value)
      : status(status), wire_size(wire_size), value(std::move(value)) {}
  ParseStatus status;
  size_t wire_size;
  String value;
};

HPackParser::String::ParseResult HPackParser::String::ParseUncompressed(
    Input* input, uint32_t length, uint32_t wire_size) {
  // Check there's enough bytes
  if (input->remaining() < length) {
    input->UnexpectedEOF(length);
    GPR_DEBUG_ASSERT(input->eof_error());
    return ParseResult{ParseStatus::kEof, wire_size, String{}};
  }
  auto* refcount = input->slice_refcount();
  auto* p = input->cur_ptr();
  input->Advance(length);
  if (refcount != nullptr) {
    return ParseResult{ParseStatus::kOk, wire_size,
                       String(refcount, p, p + length)};
  } else {
    return ParseResult{ParseStatus::kOk, wire_size,
                       String(absl::Span<const uint8_t>(p, length))};
  }
}

absl::optional<std::vector<uint8_t>> HPackParser::String::Unbase64Loop(
    const uint8_t* cur, const uint8_t* end) {
  while (cur != end && end[-1] == '=') {
    --end;
  }

  std::vector<uint8_t> out;
  out.reserve(3 * (end - cur) / 4 + 3);

  // Decode 4 bytes at a time while we can
  while (end - cur >= 4) {
    uint32_t bits = kBase64InverseTable.table[*cur];
    if (bits > 63) return {};
    uint32_t buffer = bits << 18;
    ++cur;

    bits = kBase64InverseTable.table[*cur];
    if (bits > 63) return {};
    buffer |= bits << 12;
    ++cur;

    bits = kBase64InverseTable.table[*cur];
    if (bits > 63) return {};
    buffer |= bits << 6;
    ++cur;

    bits = kBase64InverseTable.table[*cur];
    if (bits > 63) return {};
    buffer |= bits;
    ++cur;

    out.insert(out.end(), {static_cast<uint8_t>(buffer >> 16),
                           static_cast<uint8_t>(buffer >> 8),
                           static_cast<uint8_t>(buffer)});
  }
  // Deal with the last 0, 1, 2, or 3 bytes.
  switch (end - cur) {
    case 0:
      return out;
    case 1:
      return {};
    case 2: {
      uint32_t bits = kBase64InverseTable.table[*cur];
      if (bits > 63) return {};
      uint32_t buffer = bits << 18;

      ++cur;
      bits = kBase64InverseTable.table[*cur];
      if (bits > 63) return {};
      buffer |= bits << 12;

      if (buffer & 0xffff) return {};
      out.push_back(static_cast<uint8_t>(buffer >> 16));
      return out;
    }
    case 3: {
      uint32_t bits = kBase64InverseTable.table[*cur];
      if (bits > 63) return {};
      uint32_t buffer = bits << 18;

      ++cur;
      bits = kBase64InverseTable.table[*cur];
      if (bits > 63) return {};
      buffer |= bits << 12;

      ++cur;
      bits = kBase64InverseTable.table[*cur];
      if (bits > 63) return {};
      buffer |= bits << 6;

      ++cur;
      if (buffer & 0xff) return {};
      out.push_back(static_cast<uint8_t>(buffer >> 16));
      out.push_back(static_cast<uint8_t>(buffer >> 8));
      return out;
    }
  }

  GPR_UNREACHABLE_CODE(return out;);
}

HPackParser::String::ParseResult HPackParser::String::Unbase64(String s) {
  absl::optional<std::vector<uint8_t>> result;
  if (auto* p = absl::get_if<Slice>(&s.value_)) {
    result = Unbase64Loop(p->begin(), p->end());
  }
  if (auto* p = absl::get_if<absl::Span<const uint8_t>>(&s.value_)) {
    result = Unbase64Loop(p->begin(), p->end());
  }
  if (auto* p = absl::get_if<std::vector<uint8_t>>(&s.value_)) {
    result = Unbase64Loop(p->data(), p->data() + p->size());
  }
  if (!result.has_value()) {
    return ParseResult{ParseStatus::kUnbase64Failed, s.string_view().length(),
                       String{}};
  }
  return ParseResult{ParseStatus::kOk, s.string_view().length(),
                     String(std::move(*result))};
}

HPackParser::String::ParseResult HPackParser::String::Parse(Input* input,
                                                            bool is_huff,
                                                            size_t length) {
  if (is_huff) {
    // Huffman coded
    std::vector<uint8_t> output;
    ParseStatus sts =
        ParseHuff(input, length, [&output](uint8_t c) { output.push_back(c); });
    size_t wire_len = output.size();
    return ParseResult{sts, wire_len, String(std::move(output))};
  }
  return ParseUncompressed(input, length, length);
}

HPackParser::String::ParseResult HPackParser::String::ParseBinary(
    Input* input, bool is_huff, size_t length) {
  if (!is_huff) {
    if (length > 0 && input->peek() == 0) {
      // 'true-binary'
      input->Advance(1);
      return ParseUncompressed(input, length - 1, length);
    }
    // Base64 encoded... pull out the string, then unbase64 it
    auto base64 = ParseUncompressed(input, length, length);
    if (base64.status != ParseStatus::kOk) return base64;
    return Unbase64(std::move(base64.value));
  } else {
    // Huffman encoded...
    std::vector<uint8_t> decompressed;
    // State here says either we don't know if it's base64 or binary, or we do
    // and what is it.
    enum class State { kUnsure, kBinary, kBase64 };
    State state = State::kUnsure;
    auto sts = ParseHuff(input, length, [&state, &decompressed](uint8_t c) {
      if (state == State::kUnsure) {
        // First byte... if it's zero it's binary
        if (c == 0) {
          // Save the type, and skip the zero
          state = State::kBinary;
          return;
        } else {
          // Flag base64, store this value
          state = State::kBase64;
        }
      }
      // Non-first byte, or base64 first byte
      decompressed.push_back(c);
    });
    if (sts != ParseStatus::kOk) {
      return ParseResult{sts, 0, String{}};
    }
    switch (state) {
      case State::kUnsure:
        // No bytes, empty span
        return ParseResult{ParseStatus::kOk, 0,
                           String(absl::Span<const uint8_t>())};
      case State::kBinary:
        // Binary, we're done
        {
          size_t wire_len = decompressed.size();
          return ParseResult{ParseStatus::kOk, wire_len,
                             String(std::move(decompressed))};
        }
      case State::kBase64:
        // Base64 - unpack it
        return Unbase64(String(std::move(decompressed)));
    }
    GPR_UNREACHABLE_CODE(abort(););
  }
}

// Parser parses one key/value pair from a byte stream.
class HPackParser::Parser {
 public:
  Parser(Input* input, grpc_metadata_batch* metadata_buffer,
         InterSliceState& state, LogInfo log_info)
      : input_(input),
        metadata_buffer_(metadata_buffer),
        state_(state),
        log_info_(log_info) {}

  bool Parse() {
    switch (state_.parse_state) {
      case ParseState::kTop:
        return ParseTop();
      case ParseState::kParsingKeyLength:
        return ParseKeyLength();
      case ParseState::kParsingKeyBody:
        return ParseKeyBody();
      case ParseState::kSkippingKeyBody:
        return SkipKeyBody();
      case ParseState::kParsingValueLength:
        return ParseValueLength();
      case ParseState::kParsingValueBody:
        return ParseValueBody();
      case ParseState::kSkippingValueLength:
        return SkipValueLength();
      case ParseState::kSkippingValueBody:
        return SkipValueBody();
    }
    GPR_UNREACHABLE_CODE(return false);
  }

 private:
  bool ParseTop() {
    GPR_DEBUG_ASSERT(state_.parse_state == ParseState::kTop);
    auto cur = *input_->Next();
    switch (cur >> 4) {
        // Literal header not indexed - First byte format: 0000xxxx
        // Literal header never indexed - First byte format: 0001xxxx
        // Where xxxx:
        //   0000  - literal key
        //   1111  - indexed key, varint encoded index
        //   other - indexed key, inline encoded index
      case 0:
      case 1:
        switch (cur & 0xf) {
          case 0:  // literal key
            return StartParseLiteralKey(false);
          case 0xf:  // varint encoded key index
            return StartVarIdxKey(0xf, false);
          default:  // inline encoded key index
            return StartIdxKey(cur & 0xf, false);
        }
        // Update max table size.
        // First byte format: 001xxxxx
        // Where xxxxx:
        //   11111 - max size is varint encoded
        //   other - max size is stored inline
      case 2:
        // inline encoded max table size
        return FinishMaxTableSize(cur & 0x1f);
      case 3:
        if (cur == 0x3f) {
          // varint encoded max table size
          return FinishMaxTableSize(input_->ParseVarint(0x1f));
        } else {
          // inline encoded max table size
          return FinishMaxTableSize(cur & 0x1f);
        }
        // Literal header with incremental indexing.
        // First byte format: 01xxxxxx
        // Where xxxxxx:
        //   000000 - literal key
        //   111111 - indexed key, varint encoded index
        //   other  - indexed key, inline encoded index
      case 4:
        if (cur == 0x40) {
          // literal key
          return StartParseLiteralKey(true);
        }
        ABSL_FALLTHROUGH_INTENDED;
      case 5:
      case 6:
        // inline encoded key index
        return StartIdxKey(cur & 0x3f, true);
      case 7:
        if (cur == 0x7f) {
          // varint encoded key index
          return StartVarIdxKey(0x3f, true);
        } else {
          // inline encoded key index
          return StartIdxKey(cur & 0x3f, true);
        }
        // Indexed Header Field Representation
        // First byte format: 1xxxxxxx
        // Where xxxxxxx:
        //   0000000 - illegal
        //   1111111 - varint encoded field index
        //   other   - inline encoded field index
      case 8:
        if (cur == 0x80) {
          // illegal value.
          input_->SetErrorAndStopParsing(
              absl::InternalError("Illegal hpack op code"));
          return false;
        }
        ABSL_FALLTHROUGH_INTENDED;
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
        // inline encoded field index
        return FinishIndexed(cur & 0x7f);
      case 15:
        if (cur == 0xff) {
          // varint encoded field index
          return FinishIndexed(input_->ParseVarint(0x7f));
        } else {
          // inline encoded field index
          return FinishIndexed(cur & 0x7f);
        }
    }
    GPR_UNREACHABLE_CODE(abort());
  }

  void GPR_ATTRIBUTE_NOINLINE LogHeader(const HPackTable::Memento& memento) {
    const char* type;
    switch (log_info_.type) {
      case LogInfo::kHeaders:
        type = "HDR";
        break;
      case LogInfo::kTrailers:
        type = "TRL";
        break;
      case LogInfo::kDontKnow:
        type = "???";
        break;
    }
    gpr_log(GPR_DEBUG, "HTTP:%d:%s:%s: %s%s", log_info_.stream_id, type,
            log_info_.is_client ? "CLI" : "SVR",
            memento.md.DebugString().c_str(),
            memento.parse_status.ok()
                ? ""
                : absl::StrCat(
                      " (parse error: ", memento.parse_status.ToString(), ")")
                      .c_str());
  }

  void EmitHeader(const HPackTable::Memento& md) {
    // Pass up to the transport
    state_.frame_length += md.md.transport_size();
    if (!input_->has_error() &&
        state_.metadata_early_detection.MustReject(state_.frame_length)) {
      // Reject any requests above hard metadata limit.
      HandleMetadataHardSizeLimitExceeded([&md]() {
        return absl::StrCat("adding ", md.md.key(), " (length ",
                            md.md.transport_size(), "B)");
      });
    }
    if (!md.parse_status.ok()) {
      // Reject any requests with invalid metadata.
      HandleMetadataParseError(md.parse_status);
    }
    if (GPR_LIKELY(metadata_buffer_ != nullptr)) {
      metadata_buffer_->Set(md.md);
    }
  }

  bool FinishHeaderAndAddToTable(HPackTable::Memento md) {
    // Log if desired
    if (GRPC_TRACE_FLAG_ENABLED(grpc_trace_chttp2_hpack_parser)) {
      LogHeader(md);
    }
    // Emit whilst we own the metadata.
    EmitHeader(md);
    // Add to the hpack table
    grpc_error_handle err = state_.hpack_table.Add(std::move(md));
    if (GPR_UNLIKELY(!err.ok())) {
      input_->SetErrorAndStopParsing(std::move(err));
      return false;
    };
    return true;
  }

  bool FinishHeaderOmitFromTable(absl::optional<HPackTable::Memento> md) {
    // Allow higher code to just pass in failures ... simplifies things a bit.
    if (!md.has_value()) return false;
    FinishHeaderOmitFromTable(*md);
    return true;
  }

  void FinishHeaderOmitFromTable(const HPackTable::Memento& md) {
    // Log if desired
    if (GRPC_TRACE_FLAG_ENABLED(grpc_trace_chttp2_hpack_parser)) {
      LogHeader(md);
    }
    EmitHeader(md);
  }

  // Parse an index encoded key and a string encoded value
  bool StartIdxKey(uint32_t index, bool add_to_table) {
    GPR_DEBUG_ASSERT(state_.parse_state == ParseState::kTop);
    input_->UpdateFrontier();
    const auto* elem = state_.hpack_table.Lookup(index);
    if (GPR_UNLIKELY(elem == nullptr)) {
      InvalidHPackIndexError(index);
      return false;
    }
    state_.parse_state = ParseState::kParsingValueLength;
    state_.is_binary_header = elem->md.is_binary_header();
    state_.key.emplace<const HPackTable::Memento*>(elem);
    state_.add_to_table = add_to_table;
    return ParseValueLength();
  };

  // Parse a varint index encoded key and a string encoded value
  bool StartVarIdxKey(uint32_t offset, bool add_to_table) {
    auto index = input_->ParseVarint(offset);
    if (GPR_UNLIKELY(!index.has_value())) return false;
    return StartIdxKey(*index, add_to_table);
  }

  bool StartParseLiteralKey(bool add_to_table) {
    GPR_DEBUG_ASSERT(state_.parse_state == ParseState::kTop);
    state_.add_to_table = add_to_table;
    state_.parse_state = ParseState::kParsingKeyLength;
    input_->UpdateFrontier();
    return ParseKeyLength();
  }

  bool ShouldSkipParsingString(uint32_t string_length) const {
    return string_length > state_.hpack_table.current_table_size() &&
           state_.metadata_early_detection.MustReject(
               string_length + hpack_constants::kEntryOverhead);
  }

  bool ParseKeyLength() {
    GPR_DEBUG_ASSERT(state_.parse_state == ParseState::kParsingKeyLength);
    auto pfx = input_->ParseStringPrefix();
    if (!pfx.has_value()) return false;
    state_.is_string_huff_compressed = pfx->huff;
    state_.string_length = pfx->length;
    input_->UpdateFrontier();
    if (ShouldSkipParsingString(state_.string_length)) {
      HandleMetadataHardSizeLimitExceeded([pfx]() {
        return absl::StrCat("adding metadata key of length ", pfx->length);
      });
      state_.parse_state = ParseState::kSkippingKeyBody;
      return SkipKeyBody();
    } else {
      state_.parse_state = ParseState::kParsingKeyBody;
      return ParseKeyBody();
    }
  }

  bool ParseKeyBody() {
    GPR_DEBUG_ASSERT(state_.parse_state == ParseState::kParsingKeyBody);
    auto key = String::Parse(input_, state_.is_string_huff_compressed,
                             state_.string_length);
    switch (key.status) {
      case String::ParseStatus::kOk:
        break;
      case String::ParseStatus::kParseHuffFailed:
        input_->SetErrorAndStopParsing(
            absl::InternalError("Huffman decoding failed"));
        return false;
      case String::ParseStatus::kUnbase64Failed:
        Crash("unreachable");
      case String::ParseStatus::kEof:
        GPR_DEBUG_ASSERT(input_->eof_error());
        return false;
    }
    input_->UpdateFrontier();
    state_.parse_state = ParseState::kParsingValueLength;
    state_.is_binary_header = absl::EndsWith(key.value.string_view(), "-bin");
    state_.key.emplace<Slice>(key.value.Take());
    return ParseValueLength();
  }

  bool SkipStringBody() {
    auto remaining = input_->remaining();
    if (remaining >= state_.string_length) {
      input_->Advance(state_.string_length);
      return true;
    } else {
      input_->Advance(remaining);
      input_->UpdateFrontier();
      state_.string_length -= remaining;
      input_->UnexpectedEOF(std::min(state_.string_length, 1024u));
      return false;
    }
  }

  bool SkipKeyBody() {
    GPR_DEBUG_ASSERT(state_.parse_state == ParseState::kSkippingKeyBody);
    if (!SkipStringBody()) return false;
    input_->UpdateFrontier();
    state_.parse_state = ParseState::kSkippingValueLength;
    return SkipValueLength();
  }

  bool SkipValueLength() {
    GPR_DEBUG_ASSERT(state_.parse_state == ParseState::kSkippingValueLength);
    auto pfx = input_->ParseStringPrefix();
    if (!pfx.has_value()) return false;
    state_.string_length = pfx->length;
    input_->UpdateFrontier();
    state_.parse_state = ParseState::kSkippingValueBody;
    return SkipValueBody();
  }

  bool SkipValueBody() {
    GPR_DEBUG_ASSERT(state_.parse_state == ParseState::kSkippingValueBody);
    if (!SkipStringBody()) return false;
    input_->UpdateFrontier();
    state_.parse_state = ParseState::kTop;
    if (state_.add_to_table) {
      state_.hpack_table.AddLargerThanCurrentTableSize();
    }
    return true;
  }

  bool ParseValueLength() {
    GPR_DEBUG_ASSERT(state_.parse_state == ParseState::kParsingValueLength);
    auto pfx = input_->ParseStringPrefix();
    if (!pfx.has_value()) return false;
    state_.is_string_huff_compressed = pfx->huff;
    state_.string_length = pfx->length;
    input_->UpdateFrontier();
    if (ShouldSkipParsingString(state_.string_length)) {
      HandleMetadataHardSizeLimitExceeded([pfx, this]() {
        return absl::StrCat(
            "adding metadata value of length ", pfx->length, " for key \"",
            absl::CEscape(Match(
                state_.key, [](const Slice& s) { return s.as_string_view(); },
                [](const HPackTable::Memento* m) { return m->md.key(); })),
            "\"");
      });
      state_.parse_state = ParseState::kSkippingValueBody;
      return SkipValueBody();
    } else {
      state_.parse_state = ParseState::kParsingValueBody;
      return ParseValueBody();
    }
  }

  bool ParseValueBody() {
    GPR_DEBUG_ASSERT(state_.parse_state == ParseState::kParsingValueBody);
    auto value =
        state_.is_binary_header
            ? String::ParseBinary(input_, state_.is_string_huff_compressed,
                                  state_.string_length)
            : String::Parse(input_, state_.is_string_huff_compressed,
                            state_.string_length);
    absl::Status status;
    absl::string_view key_string;
    if (auto* s = absl::get_if<Slice>(&state_.key)) {
      key_string = s->as_string_view();
      status = EnsureStreamError(ValidateKey(key_string));
    } else {
      const auto* memento = absl::get<const HPackTable::Memento*>(state_.key);
      key_string = memento->md.key();
      status = memento->parse_status;
    }
    if (!status.ok()) input_->SetErrorAndContinueParsing(status);
    switch (value.status) {
      case String::ParseStatus::kOk:
        break;
      case String::ParseStatus::kParseHuffFailed:
        input_->SetErrorAndStopParsing(
            absl::InternalError("Huffman decoding failed"));
        return false;
      case String::ParseStatus::kUnbase64Failed: {
        auto error = absl::InternalError(
            absl::StrCat("Error parsing '", key_string,
                         "' metadata: illegal base64 encoding"));
        if (status.ok()) status = error;
        input_->SetErrorAndContinueParsing(error);
        break;
      }
      case String::ParseStatus::kEof:
        GPR_DEBUG_ASSERT(input_->eof_error());
        return false;
    }
    auto value_slice = value.value.Take();
    const auto transport_size =
        key_string.size() + value.wire_size + hpack_constants::kEntryOverhead;
    auto md = grpc_metadata_batch::Parse(
        key_string, std::move(value_slice), transport_size,
        [key_string, &status, this](absl::string_view msg, const Slice&) {
          auto message =
              absl::StrCat("Error parsing '", key_string, "' metadata: ", msg);
          auto error = absl::InternalError(message);
          gpr_log(GPR_ERROR, "%s", message.c_str());
          input_->SetErrorAndContinueParsing(error);
          if (status.ok()) status = error;
        });
    HPackTable::Memento memento{std::move(md), std::move(status)};
    input_->UpdateFrontier();
    state_.parse_state = ParseState::kTop;
    if (state_.add_to_table) {
      return FinishHeaderAndAddToTable(std::move(memento));
    } else {
      FinishHeaderOmitFromTable(memento);
      return true;
    }
  }

  absl::Status ValidateKey(absl::string_view key) {
    if (key == HttpSchemeMetadata::key() || key == HttpMethodMetadata::key() ||
        key == HttpAuthorityMetadata::key() || key == HttpPathMetadata::key() ||
        key == HttpStatusMetadata::key()) {
      return absl::OkStatus();
    }
    return ValidateHeaderKeyIsLegal(key);
  }

  // Emit an indexed field
  bool FinishIndexed(absl::optional<uint32_t> index) {
    state_.dynamic_table_updates_allowed = 0;
    if (!index.has_value()) return false;
    const auto* elem = state_.hpack_table.Lookup(*index);
    if (GPR_UNLIKELY(elem == nullptr)) {
      InvalidHPackIndexError(*index);
      return false;
    }
    FinishHeaderOmitFromTable(*elem);
    return true;
  }

  // finish parsing a max table size change
  bool FinishMaxTableSize(absl::optional<uint32_t> size) {
    if (!size.has_value()) return false;
    if (state_.dynamic_table_updates_allowed == 0) {
      input_->SetErrorAndStopParsing(absl::InternalError(
          "More than two max table size changes in a single frame"));
      return false;
    }
    state_.dynamic_table_updates_allowed--;
    absl::Status err = state_.hpack_table.SetCurrentTableSize(*size);
    if (!err.ok()) {
      input_->SetErrorAndStopParsing(std::move(err));
      return false;
    }
    return true;
  }

  // Set an invalid hpack index error if no error has been set. Returns result
  // unmodified.
  void InvalidHPackIndexError(uint32_t index) {
    input_->SetErrorAndStopParsing(grpc_error_set_int(
        grpc_error_set_int(absl::InternalError("Invalid HPACK index received"),
                           StatusIntProperty::kIndex,
                           static_cast<intptr_t>(index)),
        StatusIntProperty::kSize,
        static_cast<intptr_t>(state_.hpack_table.num_entries())));
  }

  GPR_ATTRIBUTE_NOINLINE
  void HandleMetadataParseError(const absl::Status& status) {
    if (metadata_buffer_ != nullptr) {
      metadata_buffer_->Clear();
      metadata_buffer_ = nullptr;
    }
    // StreamId is used as a signal to skip this stream but keep the connection
    // alive
    input_->SetErrorAndContinueParsing(status);
  }

  GPR_ATTRIBUTE_NOINLINE
  void HandleMetadataHardSizeLimitExceeded(
      absl::FunctionRef<std::string()> why) {
    // Collect a summary of sizes so far for debugging
    // Do not collect contents, for fear of exposing PII.
    std::string summary;
    std::string error_message;
    if (metadata_buffer_ != nullptr) {
      MetadataSizeLimitExceededEncoder encoder(summary);
      metadata_buffer_->Encode(&encoder);
    }
    summary = absl::StrCat("; ", why(), summary.empty() ? "" : " to ", summary);
    error_message = absl::StrCat(
        "received metadata size exceeds hard limit (", state_.frame_length,
        " vs. ", state_.metadata_early_detection.hard_limit(), ")", summary);
    HandleMetadataParseError(absl::ResourceExhaustedError(error_message));
  }

  Input* const input_;
  grpc_metadata_batch* metadata_buffer_;
  InterSliceState& state_;
  const LogInfo log_info_;
};

Slice HPackParser::String::Take() {
  if (auto* p = absl::get_if<Slice>(&value_)) {
    return p->Copy();
  } else if (auto* p = absl::get_if<absl::Span<const uint8_t>>(&value_)) {
    return Slice::FromCopiedBuffer(*p);
  } else if (auto* p = absl::get_if<std::vector<uint8_t>>(&value_)) {
    return Slice::FromCopiedBuffer(*p);
  }
  GPR_UNREACHABLE_CODE(return Slice());
}

// PUBLIC INTERFACE

HPackParser::HPackParser() = default;

HPackParser::~HPackParser() = default;

void HPackParser::BeginFrame(grpc_metadata_batch* metadata_buffer,
                             uint32_t metadata_size_soft_limit,
                             uint32_t metadata_size_hard_limit,
                             Boundary boundary, Priority priority,
                             LogInfo log_info) {
  metadata_buffer_ = metadata_buffer;
  if (metadata_buffer != nullptr) {
    metadata_buffer->Set(GrpcStatusFromWire(), true);
  }
  boundary_ = boundary;
  priority_ = priority;
  state_.dynamic_table_updates_allowed = 2;
  state_.frame_length = 0;
  state_.metadata_early_detection.ResetLimits(
      /*soft_limit=*/metadata_size_soft_limit,
      /*hard_limit=*/metadata_size_hard_limit);
  log_info_ = log_info;
}

grpc_error_handle HPackParser::Parse(const grpc_slice& slice, bool is_last) {
  if (GPR_UNLIKELY(!unparsed_bytes_.empty())) {
    unparsed_bytes_.insert(unparsed_bytes_.end(), GRPC_SLICE_START_PTR(slice),
                           GRPC_SLICE_END_PTR(slice));
    if (!is_last && unparsed_bytes_.size() < min_progress_size_) {
      // We wouldn't make progress anyway, skip out.
      return absl::OkStatus();
    }
    std::vector<uint8_t> buffer = std::move(unparsed_bytes_);
    return ParseInput(
        Input(nullptr, buffer.data(), buffer.data() + buffer.size()), is_last);
  }
  return ParseInput(Input(slice.refcount, GRPC_SLICE_START_PTR(slice),
                          GRPC_SLICE_END_PTR(slice)),
                    is_last);
}

grpc_error_handle HPackParser::ParseInput(Input input, bool is_last) {
  ParseInputInner(&input);
  if (is_last) {
    if (state_.metadata_early_detection.Reject(state_.frame_length)) {
      HandleMetadataSoftSizeLimitExceeded(&input);
    }
    global_stats().IncrementHttp2MetadataSize(state_.frame_length);
  }
  if (input.eof_error()) {
    if (GPR_UNLIKELY(is_last && is_boundary())) {
      auto err = input.TakeError();
      if (!err.ok() && !IsStreamError(err)) return err;
      return absl::InternalError(
          "Incomplete header at the end of a header/continuation sequence");
    }
    unparsed_bytes_ = std::vector<uint8_t>(input.frontier(), input.end_ptr());
    min_progress_size_ = input.min_progress_size();
    return input.TakeError();
  }
  return input.TakeError();
}

void HPackParser::ParseInputInner(Input* input) {
  switch (priority_) {
    case Priority::None:
      break;
    case Priority::Included: {
      if (input->remaining() < 5) {
        input->UnexpectedEOF(5);
        return;
      }
      input->Advance(5);
      input->UpdateFrontier();
      priority_ = Priority::None;
    }
  }
  while (!input->end_of_stream()) {
    if (GPR_UNLIKELY(
            !Parser(input, metadata_buffer_, state_, log_info_).Parse())) {
      return;
    }
    input->UpdateFrontier();
  }
}

void HPackParser::FinishFrame() { metadata_buffer_ = nullptr; }

void HPackParser::HandleMetadataSoftSizeLimitExceeded(Input* input) {
  // Collect a summary of sizes so far for debugging
  // Do not collect contents, for fear of exposing PII.
  std::string summary;
  std::string error_message;
  if (metadata_buffer_ != nullptr) {
    MetadataSizeLimitExceededEncoder encoder(summary);
    metadata_buffer_->Encode(&encoder);
  }
  error_message = absl::StrCat(
      "received metadata size exceeds soft limit (", state_.frame_length,
      " vs. ", state_.metadata_early_detection.soft_limit(),
      "), rejecting requests with some random probability", summary);
  if (metadata_buffer_ != nullptr) {
    metadata_buffer_->Clear();
    metadata_buffer_ = nullptr;
  }
  input->SetErrorAndContinueParsing(
      absl::ResourceExhaustedError(error_message));
}

}  // namespace grpc_core
