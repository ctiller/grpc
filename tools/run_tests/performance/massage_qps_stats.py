# Copyright 2017 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Autogenerated by tools/codegen/core/gen_stats_data.py

import massage_qps_stats_helpers
def massage_qps_stats(scenario_result):
  for stats in scenario_result["serverStats"] + scenario_result["clientStats"]:
    if "coreStats" in stats:
      # Get rid of the "coreStats" element and replace it by statistics
      # that correspond to columns in the bigquery schema.
      core_stats = stats["coreStats"]
      del stats["coreStats"]
      stats["core_client_calls_created"] = massage_qps_stats_helpers.counter(core_stats, "client_calls_created")
      stats["core_server_calls_created"] = massage_qps_stats_helpers.counter(core_stats, "server_calls_created")
      stats["core_client_channels_created"] = massage_qps_stats_helpers.counter(core_stats, "client_channels_created")
      stats["core_client_subchannels_created"] = massage_qps_stats_helpers.counter(core_stats, "client_subchannels_created")
      stats["core_server_channels_created"] = massage_qps_stats_helpers.counter(core_stats, "server_channels_created")
      stats["core_syscall_poll"] = massage_qps_stats_helpers.counter(core_stats, "syscall_poll")
      stats["core_pollset_kick"] = massage_qps_stats_helpers.counter(core_stats, "pollset_kick")
      stats["core_pollset_kicked_without_poller"] = massage_qps_stats_helpers.counter(core_stats, "pollset_kicked_without_poller")
      stats["core_pollset_kicked_again"] = massage_qps_stats_helpers.counter(core_stats, "pollset_kicked_again")
      stats["core_pollset_kick_wakeup_fd"] = massage_qps_stats_helpers.counter(core_stats, "pollset_kick_wakeup_fd")
      stats["core_pollset_kick_wakeup_cv"] = massage_qps_stats_helpers.counter(core_stats, "pollset_kick_wakeup_cv")
      stats["core_pollset_kick_own_thread"] = massage_qps_stats_helpers.counter(core_stats, "pollset_kick_own_thread")
      stats["core_histogram_slow_lookups"] = massage_qps_stats_helpers.counter(core_stats, "histogram_slow_lookups")
      stats["core_syscall_write"] = massage_qps_stats_helpers.counter(core_stats, "syscall_write")
      stats["core_syscall_read"] = massage_qps_stats_helpers.counter(core_stats, "syscall_read")
      stats["core_http2_settings_writes"] = massage_qps_stats_helpers.counter(core_stats, "http2_settings_writes")
      stats["core_http2_pings_sent"] = massage_qps_stats_helpers.counter(core_stats, "http2_pings_sent")
      stats["core_http2_writes_begun"] = massage_qps_stats_helpers.counter(core_stats, "http2_writes_begun")
      stats["core_http2_transport_stalls"] = massage_qps_stats_helpers.counter(core_stats, "http2_transport_stalls")
      stats["core_http2_stream_stalls"] = massage_qps_stats_helpers.counter(core_stats, "http2_stream_stalls")
      h = massage_qps_stats_helpers.histogram(core_stats, "call_initial_size")
      stats["core_call_initial_size"] = ",".join("%f" % x for x in h.buckets)
      stats["core_call_initial_size_bkts"] = ",".join("%f" % x for x in h.boundaries)
      stats["core_call_initial_size_50p"] = massage_qps_stats_helpers.percentile(h.buckets, 50, h.boundaries)
      stats["core_call_initial_size_95p"] = massage_qps_stats_helpers.percentile(h.buckets, 95, h.boundaries)
      stats["core_call_initial_size_99p"] = massage_qps_stats_helpers.percentile(h.buckets, 99, h.boundaries)
      h = massage_qps_stats_helpers.histogram(core_stats, "poll_events_returned")
      stats["core_poll_events_returned"] = ",".join("%f" % x for x in h.buckets)
      stats["core_poll_events_returned_bkts"] = ",".join("%f" % x for x in h.boundaries)
      stats["core_poll_events_returned_50p"] = massage_qps_stats_helpers.percentile(h.buckets, 50, h.boundaries)
      stats["core_poll_events_returned_95p"] = massage_qps_stats_helpers.percentile(h.buckets, 95, h.boundaries)
      stats["core_poll_events_returned_99p"] = massage_qps_stats_helpers.percentile(h.buckets, 99, h.boundaries)
      h = massage_qps_stats_helpers.histogram(core_stats, "tcp_write_size")
      stats["core_tcp_write_size"] = ",".join("%f" % x for x in h.buckets)
      stats["core_tcp_write_size_bkts"] = ",".join("%f" % x for x in h.boundaries)
      stats["core_tcp_write_size_50p"] = massage_qps_stats_helpers.percentile(h.buckets, 50, h.boundaries)
      stats["core_tcp_write_size_95p"] = massage_qps_stats_helpers.percentile(h.buckets, 95, h.boundaries)
      stats["core_tcp_write_size_99p"] = massage_qps_stats_helpers.percentile(h.buckets, 99, h.boundaries)
      h = massage_qps_stats_helpers.histogram(core_stats, "tcp_write_iov_size")
      stats["core_tcp_write_iov_size"] = ",".join("%f" % x for x in h.buckets)
      stats["core_tcp_write_iov_size_bkts"] = ",".join("%f" % x for x in h.boundaries)
      stats["core_tcp_write_iov_size_50p"] = massage_qps_stats_helpers.percentile(h.buckets, 50, h.boundaries)
      stats["core_tcp_write_iov_size_95p"] = massage_qps_stats_helpers.percentile(h.buckets, 95, h.boundaries)
      stats["core_tcp_write_iov_size_99p"] = massage_qps_stats_helpers.percentile(h.buckets, 99, h.boundaries)
      h = massage_qps_stats_helpers.histogram(core_stats, "tcp_read_allocation")
      stats["core_tcp_read_allocation"] = ",".join("%f" % x for x in h.buckets)
      stats["core_tcp_read_allocation_bkts"] = ",".join("%f" % x for x in h.boundaries)
      stats["core_tcp_read_allocation_50p"] = massage_qps_stats_helpers.percentile(h.buckets, 50, h.boundaries)
      stats["core_tcp_read_allocation_95p"] = massage_qps_stats_helpers.percentile(h.buckets, 95, h.boundaries)
      stats["core_tcp_read_allocation_99p"] = massage_qps_stats_helpers.percentile(h.buckets, 99, h.boundaries)
      h = massage_qps_stats_helpers.histogram(core_stats, "tcp_read_size")
      stats["core_tcp_read_size"] = ",".join("%f" % x for x in h.buckets)
      stats["core_tcp_read_size_bkts"] = ",".join("%f" % x for x in h.boundaries)
      stats["core_tcp_read_size_50p"] = massage_qps_stats_helpers.percentile(h.buckets, 50, h.boundaries)
      stats["core_tcp_read_size_95p"] = massage_qps_stats_helpers.percentile(h.buckets, 95, h.boundaries)
      stats["core_tcp_read_size_99p"] = massage_qps_stats_helpers.percentile(h.buckets, 99, h.boundaries)
      h = massage_qps_stats_helpers.histogram(core_stats, "tcp_read_offer")
      stats["core_tcp_read_offer"] = ",".join("%f" % x for x in h.buckets)
      stats["core_tcp_read_offer_bkts"] = ",".join("%f" % x for x in h.boundaries)
      stats["core_tcp_read_offer_50p"] = massage_qps_stats_helpers.percentile(h.buckets, 50, h.boundaries)
      stats["core_tcp_read_offer_95p"] = massage_qps_stats_helpers.percentile(h.buckets, 95, h.boundaries)
      stats["core_tcp_read_offer_99p"] = massage_qps_stats_helpers.percentile(h.buckets, 99, h.boundaries)
      h = massage_qps_stats_helpers.histogram(core_stats, "tcp_read_offer_iov_size")
      stats["core_tcp_read_offer_iov_size"] = ",".join("%f" % x for x in h.buckets)
      stats["core_tcp_read_offer_iov_size_bkts"] = ",".join("%f" % x for x in h.boundaries)
      stats["core_tcp_read_offer_iov_size_50p"] = massage_qps_stats_helpers.percentile(h.buckets, 50, h.boundaries)
      stats["core_tcp_read_offer_iov_size_95p"] = massage_qps_stats_helpers.percentile(h.buckets, 95, h.boundaries)
      stats["core_tcp_read_offer_iov_size_99p"] = massage_qps_stats_helpers.percentile(h.buckets, 99, h.boundaries)
      h = massage_qps_stats_helpers.histogram(core_stats, "http2_send_message_size")
      stats["core_http2_send_message_size"] = ",".join("%f" % x for x in h.buckets)
      stats["core_http2_send_message_size_bkts"] = ",".join("%f" % x for x in h.boundaries)
      stats["core_http2_send_message_size_50p"] = massage_qps_stats_helpers.percentile(h.buckets, 50, h.boundaries)
      stats["core_http2_send_message_size_95p"] = massage_qps_stats_helpers.percentile(h.buckets, 95, h.boundaries)
      stats["core_http2_send_message_size_99p"] = massage_qps_stats_helpers.percentile(h.buckets, 99, h.boundaries)
