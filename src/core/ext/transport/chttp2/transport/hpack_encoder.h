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

#ifndef GRPC_SRC_CORE_EXT_TRANSPORT_CHTTP2_TRANSPORT_HPACK_ENCODER_H
#define GRPC_SRC_CORE_EXT_TRANSPORT_CHTTP2_TRANSPORT_HPACK_ENCODER_H

#include <grpc/support/port_platform.h>

#include <stddef.h>

#include <cstdint>
#include <utility>
#include <vector>

#include "absl/strings/match.h"
#include "absl/strings/string_view.h"

#include <grpc/impl/compression_types.h>
#include <grpc/slice.h>
#include <grpc/status.h>

#include "src/core/ext/transport/chttp2/transport/hpack_constants.h"
#include "src/core/ext/transport/chttp2/transport/hpack_encoder_table.h"
#include "src/core/lib/compression/compression_internal.h"
#include "src/core/lib/gprpp/time.h"
#include "src/core/lib/slice/slice.h"
#include "src/core/lib/slice/slice_buffer.h"
#include "src/core/lib/transport/metadata_batch.h"
#include "src/core/lib/transport/timeout_encoding.h"
#include "src/core/lib/transport/transport.h"

namespace grpc_core {

class HPackCompressor {
  class SliceIndex;

 public:
  HPackCompressor() = default;
  ~HPackCompressor() = default;

  // Maximum table size we'll actually use.
  static constexpr uint32_t kMaxTableSize = 1024 * 1024;

  void SetMaxTableSize(uint32_t max_table_size);
  void SetMaxUsableSize(uint32_t max_table_size);

  uint32_t test_only_table_size() const {
    return table_.test_only_table_size();
  }

  struct EncodeHeaderOptions {
    uint32_t stream_id;
    bool is_end_of_stream;
    bool use_true_binary_metadata;
    size_t max_frame_size;
    grpc_transport_one_way_stats* stats;
  };

  template <typename HeaderSet>
  void EncodeHeaders(const EncodeHeaderOptions& options,
                     const HeaderSet& headers, grpc_slice_buffer* output) {
    SliceBuffer raw;
    Encoder encoder(this, options.use_true_binary_metadata, raw);
    headers.Encode(&encoder);
    Frame(options, raw, output);
  }

  template <typename HeaderSet>
  void EncodeRawHeaders(const HeaderSet& headers, SliceBuffer& output) {
    Encoder encoder(this, true, output);
    headers.Encode(&encoder);
  }

 private:
  class Encoder;

  template <typename MetadataTrait, typename CompressonTraits>
  class Compressor;

  template <typename MetadataTrait>
  class Compressor<MetadataTrait, NoCompressionCompressor> {
   public:
    void EncodeWith(MetadataTrait,
                    const typename MetadataTrait::ValueType& value,
                    Encoder* encoder) {
      const Slice& slice = MetadataValueAsSlice<MetadataTrait>(value);
      if (absl::EndsWith(MetadataTrait::key(), "-bin")) {
        encoder->EmitLitHdrWithBinaryStringKeyNotIdx(
            Slice::FromStaticString(MetadataTrait::key()), slice.Ref());
      } else {
        encoder->EmitLitHdrWithNonBinaryStringKeyNotIdx(
            Slice::FromStaticString(MetadataTrait::key()), slice.Ref());
      }
    }
  };

  template <typename MetadataTrait>
  class Compressor<MetadataTrait, FrequentKeyWithNoValueCompressionCompressor> {
   public:
    void EncodeWith(MetadataTrait,
                    const typename MetadataTrait::ValueType& value,
                    Encoder* encoder) {
      const Slice& slice = MetadataValueAsSlice<MetadataTrait>(value);
      encoder->EncodeRepeatingSliceValue(MetadataTrait::key(), slice,
                                         &some_sent_value_,
                                         HPackEncoderTable::MaxEntrySize());
    }

   private:
    // Some previously sent value with this tag.
    uint32_t some_sent_value_ = 0;
  };

  template <typename T>
  static bool IsEquivalent(T a, T b) {
    return a == b;
  }

  template <typename T>
  static bool IsEquivalent(const Slice& a, const Slice& b) {
    return a.is_equivalent(b);
  }

  template <typename MetadataTrait>
  class Compressor<MetadataTrait, StableValueCompressor> {
   public:
    void EncodeWith(MetadataTrait,
                    const typename MetadataTrait::ValueType& value,
                    Encoder* encoder) {
      auto& table = encoder->compressor()->table_;
      if (previously_sent_value_ == value &&
          table.ConvertableToDynamicIndex(previously_sent_index_)) {
        encoder->EmitIndexed(table.DynamicIndex(previously_sent_index_));
        return;
      }
      auto key = MetadataTrait::key();
      const Slice& value_slice = MetadataValueAsSlice<MetadataTrait>(value);
      if (hpack_constants::SizeForEntry(key.size(), value_slice.size()) >
          HPackEncoderTable::MaxEntrySize()) {
        encoder->EmitLitHdrWithNonBinaryStringKeyNotIdx(
            Slice::FromStaticString(key), value_slice.Ref());
        return;
      }
      encoder->EncodeAlwaysIndexed(
          &previously_sent_index_, key, value_slice.Ref(),
          hpack_constants::SizeForEntry(key.size(), value_slice.size()));
    }

   private:
    // Previously sent value
    typename MetadataTrait::ValueType previously_sent_value_{};
    // And its index in the table
    uint32_t previously_sent_index_ = 0;
  };

  template <typename MetadataTrait,
            typename MetadataTrait::ValueType known_value>
  class Compressor<
      MetadataTrait,
      KnownValueCompressor<typename MetadataTrait::ValueType, known_value>> {
   public:
    void EncodeWith(MetadataTrait,
                    const typename MetadataTrait::ValueType& value,
                    Encoder* encoder) {
      if (value != known_value) {
        gpr_log(
            GPR_ERROR, "%s",
            absl::StrCat("Not encoding bad ", MetadataTrait::key(), " header")
                .c_str());
        return;
      }
      encoder->EncodeAlwaysIndexed(
          &previously_sent_index_, MetadataTrait::key(),
          Slice(MetadataTrait::Encode(known_value)),
          MetadataTrait::key().size() +
              MetadataTrait::Encode(known_value).length() +
              hpack_constants::kEntryOverhead);
    }

   private:
    uint32_t previously_sent_index_ = 0;
  };
  template <typename MetadataTrait, size_t N>
  class Compressor<MetadataTrait, SmallIntegralValuesCompressor<N>> {
   public:
    void EncodeWith(MetadataTrait,
                    const typename MetadataTrait::ValueType& value,
                    Encoder* encoder) {
      uint32_t* index = nullptr;
      auto& table = encoder->compressor()->table_;
      if (static_cast<size_t>(value) < N) {
        index = &previously_sent_[static_cast<uint32_t>(value)];
        if (table.ConvertableToDynamicIndex(*index)) {
          encoder->EmitIndexed(table.DynamicIndex(*index));
          return;
        }
      }
      auto key = Slice::FromStaticString(MetadataTrait::key());
      auto encoded_value = MetadataTrait::Encode(value);
      size_t transport_length = key.length() + encoded_value.length() +
                                hpack_constants::kEntryOverhead;
      if (index != nullptr) {
        *index = table.AllocateIndex(transport_length);
        encoder->EmitLitHdrWithNonBinaryStringKeyIncIdx(
            std::move(key), std::move(encoded_value));
      } else {
        encoder->EmitLitHdrWithNonBinaryStringKeyNotIdx(
            std::move(key), std::move(encoded_value));
      }
    }

   private:
    uint32_t previously_sent_[N] = {};
  };

  class SliceIndex {
   public:
    void EmitTo(absl::string_view key, const Slice& value, Encoder* encoder);

   private:
    struct ValueIndex {
      ValueIndex(Slice value, uint32_t index)
          : value(std::move(value)), index(index) {}
      Slice value;
      uint32_t index;
    };
    std::vector<ValueIndex> values_;
  };

  template <typename MetadataTrait>
  class Compressor<MetadataTrait, SmallSetOfValuesCompressor> {
   public:
    void EncodeWith(MetadataTrait, const Slice& value, Encoder* encoder) {
      index_.EmitTo(MetadataTrait::key(), value, encoder);
    }

   private:
    SliceIndex index_;
  };

  struct PreviousTimeout {
    Timeout timeout;
    uint32_t index;
  };

  class TimeoutCompressorImpl {
   public:
    void EncodeWith(absl::string_view key, Timestamp deadline,
                    Encoder* encoder);

   private:
    std::vector<PreviousTimeout> previous_timeouts_;
  };

  template <typename MetadataTrait>
  class Compressor<MetadataTrait, TimeoutCompressor>
      : public TimeoutCompressorImpl {
   public:
    void EncodeWith(MetadataTrait,
                    const typename MetadataTrait::ValueType& value,
                    Encoder* encoder) {
      TimeoutCompressorImpl::EncodeWith(MetadataTrait::key(), value, encoder);
    }
  };

  template <>
  class Compressor<HttpStatusMetadata, HttpStatusCompressor> {
   public:
    void EncodeWith(HttpStatusMetadata, uint32_t value, Encoder* encoder);
  };

  template <>
  class Compressor<HttpMethodMetadata, HttpMethodCompressor> {
   public:
    void EncodeWith(HttpMethodMetadata, HttpMethodMetadata::ValueType value,
                    Encoder* encoder);
  };

  template <>
  class Compressor<HttpSchemeMetadata, HttpSchemeCompressor> {
   public:
    void EncodeWith(HttpSchemeMetadata, HttpSchemeMetadata::ValueType value,
                    Encoder* encoder);
  };

  class Encoder {
   public:
    Encoder(HPackCompressor* compressor, bool use_true_binary_metadata,
            SliceBuffer& output);

    void Encode(const Slice& key, const Slice& value);
    template <typename MetadataTrait>
    void Encode(MetadataTrait, const typename MetadataTrait::ValueType& value) {
      compressor_->compression_state_.Compressor<
          MetadataTrait, typename MetadataTrait::CompressionTraits>::
          EncodeWith(MetadataTrait(), value, this);
    }

    void AdvertiseTableSizeChange();
    void EmitIndexed(uint32_t index);
    void EmitLitHdrWithNonBinaryStringKeyIncIdx(Slice key_slice,
                                                Slice value_slice);
    void EmitLitHdrWithBinaryStringKeyIncIdx(Slice key_slice,
                                             Slice value_slice);
    void EmitLitHdrWithBinaryStringKeyNotIdx(Slice key_slice,
                                             Slice value_slice);
    void EmitLitHdrWithBinaryStringKeyNotIdx(uint32_t key_index,
                                             Slice value_slice);
    void EmitLitHdrWithNonBinaryStringKeyNotIdx(Slice key_slice,
                                                Slice value_slice);

    void EncodeAlwaysIndexed(uint32_t* index, absl::string_view key,
                             Slice value, size_t transport_length);
    void EncodeIndexedKeyWithBinaryValue(uint32_t* index, absl::string_view key,
                                         Slice value);

    void EncodeRepeatingSliceValue(const absl::string_view& key,
                                   const Slice& slice, uint32_t* index,
                                   size_t max_compression_size);

    HPackCompressor* compressor() { return compressor_; }

   private:
    const bool use_true_binary_metadata_;
    HPackCompressor* const compressor_;
    SliceBuffer& output_;
  };

  static constexpr size_t kNumFilterValues = 64;
  static constexpr uint32_t kNumCachedGrpcStatusValues = 16;

  void Frame(const EncodeHeaderOptions& options, SliceBuffer& raw,
             grpc_slice_buffer* output);

  // maximum number of bytes we'll use for the decode table (to guard against
  // peers ooming us by setting decode table size high)
  uint32_t max_usable_size_ = hpack_constants::kInitialTableSize;
  // if non-zero, advertise to the decoder that we'll start using a table
  // of this size
  bool advertise_table_size_change_ = false;
  HPackEncoderTable table_;

  using CompressionState = grpc_metadata_batch::StatefulCompressor<Compressor>;
  CompressionState compression_state_;
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_EXT_TRANSPORT_CHTTP2_TRANSPORT_HPACK_ENCODER_H
