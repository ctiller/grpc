// Copyright 2017 gRPC authors.
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

// Automatically generated by tools/codegen/core/gen_stats_data.py

#ifndef GRPC_SRC_CORE_TELEMETRY_STATS_DATA_H
#define GRPC_SRC_CORE_TELEMETRY_STATS_DATA_H

#include <grpc/support/port_platform.h>
#include <stdint.h>

#include <atomic>
#include <memory>

#include "absl/strings/string_view.h"
#include "src/core/telemetry/histogram_view.h"
#include "src/core/util/per_cpu.h"

namespace grpc_core {
class HistogramCollector_100000_20;
class Histogram_100000_20 {
 public:
  static int BucketFor(int value);
  const uint64_t* buckets() const { return buckets_; }
  size_t bucket_count() const { return 20; }
  friend Histogram_100000_20 operator-(const Histogram_100000_20& left,
                                       const Histogram_100000_20& right);

 private:
  friend class HistogramCollector_100000_20;
  uint64_t buckets_[20]{};
};
class HistogramCollector_100000_20 {
 public:
  void Increment(int value) {
    buckets_[Histogram_100000_20::BucketFor(value)].fetch_add(
        1, std::memory_order_relaxed);
  }
  void Collect(Histogram_100000_20* result) const;

 private:
  std::atomic<uint64_t> buckets_[20]{};
};
class HistogramCollector_65536_26;
class Histogram_65536_26 {
 public:
  static int BucketFor(int value);
  const uint64_t* buckets() const { return buckets_; }
  size_t bucket_count() const { return 26; }
  friend Histogram_65536_26 operator-(const Histogram_65536_26& left,
                                      const Histogram_65536_26& right);

 private:
  friend class HistogramCollector_65536_26;
  uint64_t buckets_[26]{};
};
class HistogramCollector_65536_26 {
 public:
  void Increment(int value) {
    buckets_[Histogram_65536_26::BucketFor(value)].fetch_add(
        1, std::memory_order_relaxed);
  }
  void Collect(Histogram_65536_26* result) const;

 private:
  std::atomic<uint64_t> buckets_[26]{};
};
class HistogramCollector_100_20;
class Histogram_100_20 {
 public:
  static int BucketFor(int value);
  const uint64_t* buckets() const { return buckets_; }
  size_t bucket_count() const { return 20; }
  friend Histogram_100_20 operator-(const Histogram_100_20& left,
                                    const Histogram_100_20& right);

 private:
  friend class HistogramCollector_100_20;
  uint64_t buckets_[20]{};
};
class HistogramCollector_100_20 {
 public:
  void Increment(int value) {
    buckets_[Histogram_100_20::BucketFor(value)].fetch_add(
        1, std::memory_order_relaxed);
  }
  void Collect(Histogram_100_20* result) const;

 private:
  std::atomic<uint64_t> buckets_[20]{};
};
class HistogramCollector_16777216_20;
class Histogram_16777216_20 {
 public:
  static int BucketFor(int value);
  const uint64_t* buckets() const { return buckets_; }
  size_t bucket_count() const { return 20; }
  friend Histogram_16777216_20 operator-(const Histogram_16777216_20& left,
                                         const Histogram_16777216_20& right);

 private:
  friend class HistogramCollector_16777216_20;
  uint64_t buckets_[20]{};
};
class HistogramCollector_16777216_20 {
 public:
  void Increment(int value) {
    buckets_[Histogram_16777216_20::BucketFor(value)].fetch_add(
        1, std::memory_order_relaxed);
  }
  void Collect(Histogram_16777216_20* result) const;

 private:
  std::atomic<uint64_t> buckets_[20]{};
};
class HistogramCollector_80_10;
class Histogram_80_10 {
 public:
  static int BucketFor(int value);
  const uint64_t* buckets() const { return buckets_; }
  size_t bucket_count() const { return 10; }
  friend Histogram_80_10 operator-(const Histogram_80_10& left,
                                   const Histogram_80_10& right);

 private:
  friend class HistogramCollector_80_10;
  uint64_t buckets_[10]{};
};
class HistogramCollector_80_10 {
 public:
  void Increment(int value) {
    buckets_[Histogram_80_10::BucketFor(value)].fetch_add(
        1, std::memory_order_relaxed);
  }
  void Collect(Histogram_80_10* result) const;

 private:
  std::atomic<uint64_t> buckets_[10]{};
};
class HistogramCollector_10000_20;
class Histogram_10000_20 {
 public:
  static int BucketFor(int value);
  const uint64_t* buckets() const { return buckets_; }
  size_t bucket_count() const { return 20; }
  friend Histogram_10000_20 operator-(const Histogram_10000_20& left,
                                      const Histogram_10000_20& right);

 private:
  friend class HistogramCollector_10000_20;
  uint64_t buckets_[20]{};
};
class HistogramCollector_10000_20 {
 public:
  void Increment(int value) {
    buckets_[Histogram_10000_20::BucketFor(value)].fetch_add(
        1, std::memory_order_relaxed);
  }
  void Collect(Histogram_10000_20* result) const;

 private:
  std::atomic<uint64_t> buckets_[20]{};
};
class HistogramCollector_1800000_40;
class Histogram_1800000_40 {
 public:
  static int BucketFor(int value);
  const uint64_t* buckets() const { return buckets_; }
  size_t bucket_count() const { return 40; }
  friend Histogram_1800000_40 operator-(const Histogram_1800000_40& left,
                                        const Histogram_1800000_40& right);

 private:
  friend class HistogramCollector_1800000_40;
  uint64_t buckets_[40]{};
};
class HistogramCollector_1800000_40 {
 public:
  void Increment(int value) {
    buckets_[Histogram_1800000_40::BucketFor(value)].fetch_add(
        1, std::memory_order_relaxed);
  }
  void Collect(Histogram_1800000_40* result) const;

 private:
  std::atomic<uint64_t> buckets_[40]{};
};
struct GlobalStats {
  enum class Counter {
    kClientCallsCreated,
    kServerCallsCreated,
    kClientChannelsCreated,
    kClientSubchannelsCreated,
    kServerChannelsCreated,
    kInsecureConnectionsCreated,
    kRqConnectionsDropped,
    kRqCallsDropped,
    kRqCallsRejected,
    kSyscallWrite,
    kSyscallRead,
    kTcpReadAlloc8k,
    kTcpReadAlloc64k,
    kHttp2SettingsWrites,
    kHttp2PingsSent,
    kHttp2WritesBegun,
    kHttp2TransportStalls,
    kHttp2StreamStalls,
    kHttp2HpackHits,
    kHttp2HpackMisses,
    kCqPluckCreates,
    kCqNextCreates,
    kCqCallbackCreates,
    kWrrUpdates,
    kWorkSerializerItemsEnqueued,
    kWorkSerializerItemsDequeued,
    kEconnabortedCount,
    kEconnresetCount,
    kEpipeCount,
    kEtimedoutCount,
    kEconnrefusedCount,
    kEnetunreachCount,
    kEnomsgCount,
    kEnotconnCount,
    kEnobufsCount,
    kUncommonIoErrorCount,
    kMsgErrqueueErrorCount,
    COUNT
  };
  enum class Histogram {
    kCallInitialSize,
    kTcpWriteSize,
    kTcpWriteIovSize,
    kTcpReadSize,
    kTcpReadOffer,
    kTcpReadOfferIovSize,
    kHttp2SendMessageSize,
    kHttp2MetadataSize,
    kHttp2HpackEntryLifetime,
    kHttp2HeaderTableSize,
    kHttp2InitialWindowSize,
    kHttp2MaxConcurrentStreams,
    kHttp2MaxFrameSize,
    kHttp2MaxHeaderListSize,
    kHttp2PreferredReceiveCryptoMessageSize,
    kHttp2StreamRemoteWindowUpdate,
    kHttp2TransportRemoteWindowUpdate,
    kHttp2TransportWindowUpdatePeriod,
    kHttp2StreamWindowUpdatePeriod,
    kHttp2WriteTargetSize,
    kHttp2WriteDataFrameSize,
    kHttp2ReadDataFrameSize,
    kWrrSubchannelListSize,
    kWrrSubchannelReadySize,
    kWorkSerializerRunTimeMs,
    kWorkSerializerWorkTimeMs,
    kWorkSerializerWorkTimePerItemMs,
    kWorkSerializerItemsPerRun,
    kChaoticGoodSendmsgsPerWriteControl,
    kChaoticGoodRecvmsgsPerReadControl,
    kChaoticGoodSendmsgsPerWriteData,
    kChaoticGoodRecvmsgsPerReadData,
    kChaoticGoodThreadHopsPerWriteControl,
    kChaoticGoodThreadHopsPerReadControl,
    kChaoticGoodThreadHopsPerWriteData,
    kChaoticGoodThreadHopsPerReadData,
    kChaoticGoodTcpReadSizeData,
    kChaoticGoodTcpReadSizeControl,
    kChaoticGoodTcpReadOfferData,
    kChaoticGoodTcpReadOfferControl,
    kChaoticGoodTcpWriteSizeData,
    kChaoticGoodTcpWriteSizeControl,
    COUNT
  };
  GlobalStats();
  static const absl::string_view counter_name[static_cast<int>(Counter::COUNT)];
  static const absl::string_view
      histogram_name[static_cast<int>(Histogram::COUNT)];
  static const absl::string_view counter_doc[static_cast<int>(Counter::COUNT)];
  static const absl::string_view
      histogram_doc[static_cast<int>(Histogram::COUNT)];
  union {
    struct {
      uint64_t client_calls_created;
      uint64_t server_calls_created;
      uint64_t client_channels_created;
      uint64_t client_subchannels_created;
      uint64_t server_channels_created;
      uint64_t insecure_connections_created;
      uint64_t rq_connections_dropped;
      uint64_t rq_calls_dropped;
      uint64_t rq_calls_rejected;
      uint64_t syscall_write;
      uint64_t syscall_read;
      uint64_t tcp_read_alloc_8k;
      uint64_t tcp_read_alloc_64k;
      uint64_t http2_settings_writes;
      uint64_t http2_pings_sent;
      uint64_t http2_writes_begun;
      uint64_t http2_transport_stalls;
      uint64_t http2_stream_stalls;
      uint64_t http2_hpack_hits;
      uint64_t http2_hpack_misses;
      uint64_t cq_pluck_creates;
      uint64_t cq_next_creates;
      uint64_t cq_callback_creates;
      uint64_t wrr_updates;
      uint64_t work_serializer_items_enqueued;
      uint64_t work_serializer_items_dequeued;
      uint64_t econnaborted_count;
      uint64_t econnreset_count;
      uint64_t epipe_count;
      uint64_t etimedout_count;
      uint64_t econnrefused_count;
      uint64_t enetunreach_count;
      uint64_t enomsg_count;
      uint64_t enotconn_count;
      uint64_t enobufs_count;
      uint64_t uncommon_io_error_count;
      uint64_t msg_errqueue_error_count;
    };
    uint64_t counters[static_cast<int>(Counter::COUNT)];
  };
  Histogram_65536_26 call_initial_size;
  Histogram_16777216_20 tcp_write_size;
  Histogram_80_10 tcp_write_iov_size;
  Histogram_16777216_20 tcp_read_size;
  Histogram_16777216_20 tcp_read_offer;
  Histogram_80_10 tcp_read_offer_iov_size;
  Histogram_16777216_20 http2_send_message_size;
  Histogram_65536_26 http2_metadata_size;
  Histogram_1800000_40 http2_hpack_entry_lifetime;
  Histogram_16777216_20 http2_header_table_size;
  Histogram_16777216_20 http2_initial_window_size;
  Histogram_16777216_20 http2_max_concurrent_streams;
  Histogram_16777216_20 http2_max_frame_size;
  Histogram_16777216_20 http2_max_header_list_size;
  Histogram_16777216_20 http2_preferred_receive_crypto_message_size;
  Histogram_16777216_20 http2_stream_remote_window_update;
  Histogram_16777216_20 http2_transport_remote_window_update;
  Histogram_100000_20 http2_transport_window_update_period;
  Histogram_100000_20 http2_stream_window_update_period;
  Histogram_16777216_20 http2_write_target_size;
  Histogram_16777216_20 http2_write_data_frame_size;
  Histogram_16777216_20 http2_read_data_frame_size;
  Histogram_10000_20 wrr_subchannel_list_size;
  Histogram_10000_20 wrr_subchannel_ready_size;
  Histogram_100000_20 work_serializer_run_time_ms;
  Histogram_100000_20 work_serializer_work_time_ms;
  Histogram_100000_20 work_serializer_work_time_per_item_ms;
  Histogram_10000_20 work_serializer_items_per_run;
  Histogram_100_20 chaotic_good_sendmsgs_per_write_control;
  Histogram_100_20 chaotic_good_recvmsgs_per_read_control;
  Histogram_100_20 chaotic_good_sendmsgs_per_write_data;
  Histogram_100_20 chaotic_good_recvmsgs_per_read_data;
  Histogram_100_20 chaotic_good_thread_hops_per_write_control;
  Histogram_100_20 chaotic_good_thread_hops_per_read_control;
  Histogram_100_20 chaotic_good_thread_hops_per_write_data;
  Histogram_100_20 chaotic_good_thread_hops_per_read_data;
  Histogram_16777216_20 chaotic_good_tcp_read_size_data;
  Histogram_16777216_20 chaotic_good_tcp_read_size_control;
  Histogram_16777216_20 chaotic_good_tcp_read_offer_data;
  Histogram_16777216_20 chaotic_good_tcp_read_offer_control;
  Histogram_16777216_20 chaotic_good_tcp_write_size_data;
  Histogram_16777216_20 chaotic_good_tcp_write_size_control;
  HistogramView histogram(Histogram which) const;
  std::unique_ptr<GlobalStats> Diff(const GlobalStats& other) const;
};
class GlobalStatsCollector {
 public:
  std::unique_ptr<GlobalStats> Collect() const;
  void IncrementClientCallsCreated() {
    data_.this_cpu().client_calls_created.fetch_add(1,
                                                    std::memory_order_relaxed);
  }
  void IncrementServerCallsCreated() {
    data_.this_cpu().server_calls_created.fetch_add(1,
                                                    std::memory_order_relaxed);
  }
  void IncrementClientChannelsCreated() {
    data_.this_cpu().client_channels_created.fetch_add(
        1, std::memory_order_relaxed);
  }
  void IncrementClientSubchannelsCreated() {
    data_.this_cpu().client_subchannels_created.fetch_add(
        1, std::memory_order_relaxed);
  }
  void IncrementServerChannelsCreated() {
    data_.this_cpu().server_channels_created.fetch_add(
        1, std::memory_order_relaxed);
  }
  void IncrementInsecureConnectionsCreated() {
    data_.this_cpu().insecure_connections_created.fetch_add(
        1, std::memory_order_relaxed);
  }
  void IncrementRqConnectionsDropped() {
    data_.this_cpu().rq_connections_dropped.fetch_add(
        1, std::memory_order_relaxed);
  }
  void IncrementRqCallsDropped() {
    data_.this_cpu().rq_calls_dropped.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementRqCallsRejected() {
    data_.this_cpu().rq_calls_rejected.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementSyscallWrite() {
    data_.this_cpu().syscall_write.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementSyscallRead() {
    data_.this_cpu().syscall_read.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementTcpReadAlloc8k() {
    data_.this_cpu().tcp_read_alloc_8k.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementTcpReadAlloc64k() {
    data_.this_cpu().tcp_read_alloc_64k.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementHttp2SettingsWrites() {
    data_.this_cpu().http2_settings_writes.fetch_add(1,
                                                     std::memory_order_relaxed);
  }
  void IncrementHttp2PingsSent() {
    data_.this_cpu().http2_pings_sent.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementHttp2WritesBegun() {
    data_.this_cpu().http2_writes_begun.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementHttp2TransportStalls() {
    data_.this_cpu().http2_transport_stalls.fetch_add(
        1, std::memory_order_relaxed);
  }
  void IncrementHttp2StreamStalls() {
    data_.this_cpu().http2_stream_stalls.fetch_add(1,
                                                   std::memory_order_relaxed);
  }
  void IncrementHttp2HpackHits() {
    data_.this_cpu().http2_hpack_hits.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementHttp2HpackMisses() {
    data_.this_cpu().http2_hpack_misses.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementCqPluckCreates() {
    data_.this_cpu().cq_pluck_creates.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementCqNextCreates() {
    data_.this_cpu().cq_next_creates.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementCqCallbackCreates() {
    data_.this_cpu().cq_callback_creates.fetch_add(1,
                                                   std::memory_order_relaxed);
  }
  void IncrementWrrUpdates() {
    data_.this_cpu().wrr_updates.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementWorkSerializerItemsEnqueued() {
    data_.this_cpu().work_serializer_items_enqueued.fetch_add(
        1, std::memory_order_relaxed);
  }
  void IncrementWorkSerializerItemsDequeued() {
    data_.this_cpu().work_serializer_items_dequeued.fetch_add(
        1, std::memory_order_relaxed);
  }
  void IncrementEconnabortedCount() {
    data_.this_cpu().econnaborted_count.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementEconnresetCount() {
    data_.this_cpu().econnreset_count.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementEpipeCount() {
    data_.this_cpu().epipe_count.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementEtimedoutCount() {
    data_.this_cpu().etimedout_count.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementEconnrefusedCount() {
    data_.this_cpu().econnrefused_count.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementEnetunreachCount() {
    data_.this_cpu().enetunreach_count.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementEnomsgCount() {
    data_.this_cpu().enomsg_count.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementEnotconnCount() {
    data_.this_cpu().enotconn_count.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementEnobufsCount() {
    data_.this_cpu().enobufs_count.fetch_add(1, std::memory_order_relaxed);
  }
  void IncrementUncommonIoErrorCount() {
    data_.this_cpu().uncommon_io_error_count.fetch_add(
        1, std::memory_order_relaxed);
  }
  void IncrementMsgErrqueueErrorCount() {
    data_.this_cpu().msg_errqueue_error_count.fetch_add(
        1, std::memory_order_relaxed);
  }
  void IncrementCallInitialSize(int value) {
    data_.this_cpu().call_initial_size.Increment(value);
  }
  void IncrementTcpWriteSize(int value) {
    data_.this_cpu().tcp_write_size.Increment(value);
  }
  void IncrementTcpWriteIovSize(int value) {
    data_.this_cpu().tcp_write_iov_size.Increment(value);
  }
  void IncrementTcpReadSize(int value) {
    data_.this_cpu().tcp_read_size.Increment(value);
  }
  void IncrementTcpReadOffer(int value) {
    data_.this_cpu().tcp_read_offer.Increment(value);
  }
  void IncrementTcpReadOfferIovSize(int value) {
    data_.this_cpu().tcp_read_offer_iov_size.Increment(value);
  }
  void IncrementHttp2SendMessageSize(int value) {
    data_.this_cpu().http2_send_message_size.Increment(value);
  }
  void IncrementHttp2MetadataSize(int value) {
    data_.this_cpu().http2_metadata_size.Increment(value);
  }
  void IncrementHttp2HpackEntryLifetime(int value) {
    data_.this_cpu().http2_hpack_entry_lifetime.Increment(value);
  }
  void IncrementHttp2HeaderTableSize(int value) {
    data_.this_cpu().http2_header_table_size.Increment(value);
  }
  void IncrementHttp2InitialWindowSize(int value) {
    data_.this_cpu().http2_initial_window_size.Increment(value);
  }
  void IncrementHttp2MaxConcurrentStreams(int value) {
    data_.this_cpu().http2_max_concurrent_streams.Increment(value);
  }
  void IncrementHttp2MaxFrameSize(int value) {
    data_.this_cpu().http2_max_frame_size.Increment(value);
  }
  void IncrementHttp2MaxHeaderListSize(int value) {
    data_.this_cpu().http2_max_header_list_size.Increment(value);
  }
  void IncrementHttp2PreferredReceiveCryptoMessageSize(int value) {
    data_.this_cpu().http2_preferred_receive_crypto_message_size.Increment(
        value);
  }
  void IncrementHttp2StreamRemoteWindowUpdate(int value) {
    data_.this_cpu().http2_stream_remote_window_update.Increment(value);
  }
  void IncrementHttp2TransportRemoteWindowUpdate(int value) {
    data_.this_cpu().http2_transport_remote_window_update.Increment(value);
  }
  void IncrementHttp2TransportWindowUpdatePeriod(int value) {
    data_.this_cpu().http2_transport_window_update_period.Increment(value);
  }
  void IncrementHttp2StreamWindowUpdatePeriod(int value) {
    data_.this_cpu().http2_stream_window_update_period.Increment(value);
  }
  void IncrementHttp2WriteTargetSize(int value) {
    data_.this_cpu().http2_write_target_size.Increment(value);
  }
  void IncrementHttp2WriteDataFrameSize(int value) {
    data_.this_cpu().http2_write_data_frame_size.Increment(value);
  }
  void IncrementHttp2ReadDataFrameSize(int value) {
    data_.this_cpu().http2_read_data_frame_size.Increment(value);
  }
  void IncrementWrrSubchannelListSize(int value) {
    data_.this_cpu().wrr_subchannel_list_size.Increment(value);
  }
  void IncrementWrrSubchannelReadySize(int value) {
    data_.this_cpu().wrr_subchannel_ready_size.Increment(value);
  }
  void IncrementWorkSerializerRunTimeMs(int value) {
    data_.this_cpu().work_serializer_run_time_ms.Increment(value);
  }
  void IncrementWorkSerializerWorkTimeMs(int value) {
    data_.this_cpu().work_serializer_work_time_ms.Increment(value);
  }
  void IncrementWorkSerializerWorkTimePerItemMs(int value) {
    data_.this_cpu().work_serializer_work_time_per_item_ms.Increment(value);
  }
  void IncrementWorkSerializerItemsPerRun(int value) {
    data_.this_cpu().work_serializer_items_per_run.Increment(value);
  }
  void IncrementChaoticGoodSendmsgsPerWriteControl(int value) {
    data_.this_cpu().chaotic_good_sendmsgs_per_write_control.Increment(value);
  }
  void IncrementChaoticGoodRecvmsgsPerReadControl(int value) {
    data_.this_cpu().chaotic_good_recvmsgs_per_read_control.Increment(value);
  }
  void IncrementChaoticGoodSendmsgsPerWriteData(int value) {
    data_.this_cpu().chaotic_good_sendmsgs_per_write_data.Increment(value);
  }
  void IncrementChaoticGoodRecvmsgsPerReadData(int value) {
    data_.this_cpu().chaotic_good_recvmsgs_per_read_data.Increment(value);
  }
  void IncrementChaoticGoodThreadHopsPerWriteControl(int value) {
    data_.this_cpu().chaotic_good_thread_hops_per_write_control.Increment(
        value);
  }
  void IncrementChaoticGoodThreadHopsPerReadControl(int value) {
    data_.this_cpu().chaotic_good_thread_hops_per_read_control.Increment(value);
  }
  void IncrementChaoticGoodThreadHopsPerWriteData(int value) {
    data_.this_cpu().chaotic_good_thread_hops_per_write_data.Increment(value);
  }
  void IncrementChaoticGoodThreadHopsPerReadData(int value) {
    data_.this_cpu().chaotic_good_thread_hops_per_read_data.Increment(value);
  }
  void IncrementChaoticGoodTcpReadSizeData(int value) {
    data_.this_cpu().chaotic_good_tcp_read_size_data.Increment(value);
  }
  void IncrementChaoticGoodTcpReadSizeControl(int value) {
    data_.this_cpu().chaotic_good_tcp_read_size_control.Increment(value);
  }
  void IncrementChaoticGoodTcpReadOfferData(int value) {
    data_.this_cpu().chaotic_good_tcp_read_offer_data.Increment(value);
  }
  void IncrementChaoticGoodTcpReadOfferControl(int value) {
    data_.this_cpu().chaotic_good_tcp_read_offer_control.Increment(value);
  }
  void IncrementChaoticGoodTcpWriteSizeData(int value) {
    data_.this_cpu().chaotic_good_tcp_write_size_data.Increment(value);
  }
  void IncrementChaoticGoodTcpWriteSizeControl(int value) {
    data_.this_cpu().chaotic_good_tcp_write_size_control.Increment(value);
  }

 private:
  struct Data {
    std::atomic<uint64_t> client_calls_created{0};
    std::atomic<uint64_t> server_calls_created{0};
    std::atomic<uint64_t> client_channels_created{0};
    std::atomic<uint64_t> client_subchannels_created{0};
    std::atomic<uint64_t> server_channels_created{0};
    std::atomic<uint64_t> insecure_connections_created{0};
    std::atomic<uint64_t> rq_connections_dropped{0};
    std::atomic<uint64_t> rq_calls_dropped{0};
    std::atomic<uint64_t> rq_calls_rejected{0};
    std::atomic<uint64_t> syscall_write{0};
    std::atomic<uint64_t> syscall_read{0};
    std::atomic<uint64_t> tcp_read_alloc_8k{0};
    std::atomic<uint64_t> tcp_read_alloc_64k{0};
    std::atomic<uint64_t> http2_settings_writes{0};
    std::atomic<uint64_t> http2_pings_sent{0};
    std::atomic<uint64_t> http2_writes_begun{0};
    std::atomic<uint64_t> http2_transport_stalls{0};
    std::atomic<uint64_t> http2_stream_stalls{0};
    std::atomic<uint64_t> http2_hpack_hits{0};
    std::atomic<uint64_t> http2_hpack_misses{0};
    std::atomic<uint64_t> cq_pluck_creates{0};
    std::atomic<uint64_t> cq_next_creates{0};
    std::atomic<uint64_t> cq_callback_creates{0};
    std::atomic<uint64_t> wrr_updates{0};
    std::atomic<uint64_t> work_serializer_items_enqueued{0};
    std::atomic<uint64_t> work_serializer_items_dequeued{0};
    std::atomic<uint64_t> econnaborted_count{0};
    std::atomic<uint64_t> econnreset_count{0};
    std::atomic<uint64_t> epipe_count{0};
    std::atomic<uint64_t> etimedout_count{0};
    std::atomic<uint64_t> econnrefused_count{0};
    std::atomic<uint64_t> enetunreach_count{0};
    std::atomic<uint64_t> enomsg_count{0};
    std::atomic<uint64_t> enotconn_count{0};
    std::atomic<uint64_t> enobufs_count{0};
    std::atomic<uint64_t> uncommon_io_error_count{0};
    std::atomic<uint64_t> msg_errqueue_error_count{0};
    HistogramCollector_65536_26 call_initial_size;
    HistogramCollector_16777216_20 tcp_write_size;
    HistogramCollector_80_10 tcp_write_iov_size;
    HistogramCollector_16777216_20 tcp_read_size;
    HistogramCollector_16777216_20 tcp_read_offer;
    HistogramCollector_80_10 tcp_read_offer_iov_size;
    HistogramCollector_16777216_20 http2_send_message_size;
    HistogramCollector_65536_26 http2_metadata_size;
    HistogramCollector_1800000_40 http2_hpack_entry_lifetime;
    HistogramCollector_16777216_20 http2_header_table_size;
    HistogramCollector_16777216_20 http2_initial_window_size;
    HistogramCollector_16777216_20 http2_max_concurrent_streams;
    HistogramCollector_16777216_20 http2_max_frame_size;
    HistogramCollector_16777216_20 http2_max_header_list_size;
    HistogramCollector_16777216_20 http2_preferred_receive_crypto_message_size;
    HistogramCollector_16777216_20 http2_stream_remote_window_update;
    HistogramCollector_16777216_20 http2_transport_remote_window_update;
    HistogramCollector_100000_20 http2_transport_window_update_period;
    HistogramCollector_100000_20 http2_stream_window_update_period;
    HistogramCollector_16777216_20 http2_write_target_size;
    HistogramCollector_16777216_20 http2_write_data_frame_size;
    HistogramCollector_16777216_20 http2_read_data_frame_size;
    HistogramCollector_10000_20 wrr_subchannel_list_size;
    HistogramCollector_10000_20 wrr_subchannel_ready_size;
    HistogramCollector_100000_20 work_serializer_run_time_ms;
    HistogramCollector_100000_20 work_serializer_work_time_ms;
    HistogramCollector_100000_20 work_serializer_work_time_per_item_ms;
    HistogramCollector_10000_20 work_serializer_items_per_run;
    HistogramCollector_100_20 chaotic_good_sendmsgs_per_write_control;
    HistogramCollector_100_20 chaotic_good_recvmsgs_per_read_control;
    HistogramCollector_100_20 chaotic_good_sendmsgs_per_write_data;
    HistogramCollector_100_20 chaotic_good_recvmsgs_per_read_data;
    HistogramCollector_100_20 chaotic_good_thread_hops_per_write_control;
    HistogramCollector_100_20 chaotic_good_thread_hops_per_read_control;
    HistogramCollector_100_20 chaotic_good_thread_hops_per_write_data;
    HistogramCollector_100_20 chaotic_good_thread_hops_per_read_data;
    HistogramCollector_16777216_20 chaotic_good_tcp_read_size_data;
    HistogramCollector_16777216_20 chaotic_good_tcp_read_size_control;
    HistogramCollector_16777216_20 chaotic_good_tcp_read_offer_data;
    HistogramCollector_16777216_20 chaotic_good_tcp_read_offer_control;
    HistogramCollector_16777216_20 chaotic_good_tcp_write_size_data;
    HistogramCollector_16777216_20 chaotic_good_tcp_write_size_control;
  };
  PerCpu<Data> data_{PerCpuOptions().SetCpusPerShard(4).SetMaxShards(32)};
};
}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_TELEMETRY_STATS_DATA_H
