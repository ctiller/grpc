# Copyright 2022 gRPC authors.
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
"""A list of flaky tests, consumed by generate_tests.bzl to set flaky attrs."""
FLAKY_TESTS = [
    "h2_census_test@bad_ping",
    "h2_census_test@grpc_authz",
    "h2_census_test@retry_per_attempt_recv_timeout",
    "h2_census_test@retry_too_many_attempts",
    "h2_census_test@retry_transparent_mcs",
    "h2_census_test@shutdown_finishes_calls",
    "h2_compress_test@grpc_authz",
    "h2_compress_test@keepalive_timeout",
    "h2_compress_test@max_connection_age",
    "h2_compress_test@max_connection_idle",
    "h2_compress_test@retry_cancellation",
    "h2_fakesec_test@retry_per_attempt_recv_timeout",
    "h2_fakesec_test@retry_server_pushback_disabled",
    "h2_fakesec_test@retry_too_many_attempts",
    "h2_fakesec_test@shutdown_finishes_calls",
    "h2_fd_test@max_connection_age",
    "h2_full+pipe_test@max_connection_idle",
    "h2_full_no_retry_test@bad_ping",
    "h2_full_no_retry_test@grpc_authz",
    "h2_full_no_retry_test@hpack_size",
    "h2_full_no_retry_test@max_connection_age",
    "h2_full_no_retry_test@max_connection_idle",
    "h2_full_test@bad_ping",
    "h2_full_test@keepalive_timeout",
    "h2_full_test@max_connection_age",
    "h2_full_test@max_connection_idle",
    "h2_full_test@retry_per_attempt_recv_timeout",
    "h2_full_test@retry_recv_trailing_metadata_error",
    "h2_full_test@retry_send_initial_metadata_refs",
    "h2_full_test@retry_server_pushback_delay",
    "h2_http_proxy_test@compressed_payload",
    "h2_http_proxy_test@connectivity",
    "h2_http_proxy_test@grpc_authz",
    "h2_http_proxy_test@hpack_size",
    "h2_http_proxy_test@max_connection_age",
    "h2_http_proxy_test@max_connection_idle",
    "h2_http_proxy_test@ping_pong_streaming",
    "h2_http_proxy_test@retry_per_attempt_recv_timeout",
    "h2_insecure_test@bad_ping",
    "h2_insecure_test@grpc_authz",
    "h2_insecure_test@max_connection_age",
    "h2_insecure_test@max_connection_idle",
    "h2_insecure_test@retry",
    "h2_insecure_test@retry_send_initial_metadata_refs",
    "h2_insecure_test@retry_server_pushback_disabled",
    "h2_local_abstract_uds_percent_encoded_test@max_connection_idle",
    "h2_local_ipv4_test@bad_ping",
    "h2_local_ipv4_test@max_connection_idle",
    "h2_local_ipv4_test@retry",
    "h2_local_ipv4_test@retry_per_attempt_recv_timeout",
    "h2_local_ipv4_test@retry_send_initial_metadata_refs",
    "h2_local_ipv4_test@retry_streaming2",
    "h2_local_ipv6_test@bad_ping",
    "h2_local_ipv6_test@cancel_after_accept",
    "h2_local_ipv6_test@max_connection_idle",
    "h2_local_ipv6_test@retry_per_attempt_recv_timeout",
    "h2_local_ipv6_test@retry_server_pushback_disabled",
    "h2_local_ipv6_test@shutdown_finishes_calls",
    "h2_local_uds_percent_encoded_test@bad_ping",
    "h2_local_uds_percent_encoded_test@connectivity",
    "h2_local_uds_percent_encoded_test@max_connection_idle",
    "h2_local_uds_percent_encoded_test@retry_per_attempt_recv_timeout",
    "h2_local_uds_percent_encoded_test@retry_streaming2",
    "h2_local_uds_percent_encoded_test@retry_transparent_mcs",
    "h2_local_uds_test@bad_ping",
    "h2_local_uds_test@grpc_authz",
    "h2_local_uds_test@max_connection_age",
    "h2_local_uds_test@max_connection_idle",
    "h2_local_uds_test@retry_server_pushback_disabled",
    "h2_oauth2_test@bad_ping",
    "h2_oauth2_test@connectivity",
    "h2_oauth2_test@filter_context",
    "h2_oauth2_test@grpc_authz",
    "h2_oauth2_test@max_connection_age",
    "h2_oauth2_test@max_connection_idle",
    "h2_oauth2_test@retry_server_pushback_delay",
    "h2_oauth2_test@retry_server_pushback_disabled",
    "h2_oauth2_test@retry_too_many_attempts",
    "h2_proxy_test@grpc_authz",
    "h2_proxy_test@retry_send_initial_metadata_refs",
    "h2_proxy_test@shutdown_finishes_calls",
    "h2_sockpair_1byte_test@max_connection_age",
    "h2_sockpair_1byte_test@shutdown_finishes_calls",
    "h2_sockpair_test@grpc_authz",
    "h2_sockpair_test@max_connection_age",
    "h2_ssl_cred_reload_test@bad_ping",
    "h2_ssl_cred_reload_test@client_streaming",
    "h2_ssl_cred_reload_test@connectivity",
    "h2_ssl_cred_reload_test@max_connection_idle",
    "h2_ssl_cred_reload_test@retry",
    "h2_ssl_cred_reload_test@retry_per_attempt_recv_timeout",
    "h2_ssl_cred_reload_test@retry_server_pushback_delay",
    "h2_ssl_cred_reload_test@retry_too_many_attempts",
    "h2_ssl_cred_reload_test@simple_delayed_request",
    "h2_ssl_proxy_test@disappearing_server",
    "h2_ssl_proxy_test@retry_cancellation",
    "h2_ssl_proxy_test@retry_server_pushback_delay",
    "h2_ssl_proxy_test@retry_too_many_attempts",
    "h2_ssl_proxy_test@shutdown_finishes_calls",
    "h2_ssl_test@bad_ping",
    "h2_ssl_test@compressed_payload",
    "h2_ssl_test@grpc_authz",
    "h2_ssl_test@max_connection_idle",
    "h2_ssl_test@retry",
    "h2_ssl_test@retry_cancellation",
    "h2_ssl_test@retry_per_attempt_recv_timeout",
    "h2_ssl_test@retry_send_initial_metadata_refs",
    "h2_ssl_test@retry_server_pushback_disabled",
    "h2_ssl_test@retry_too_many_attempts",
    "h2_ssl_test@shutdown_finishes_calls",
    "h2_tls_certwatch_async_tls1_3_test@cancel_after_invoke",
    "h2_tls_certwatch_async_tls1_3_test@connectivity",
    "h2_tls_certwatch_async_tls1_3_test@max_connection_idle",
    "h2_tls_certwatch_async_tls1_3_test@retry_cancellation",
    "h2_tls_certwatch_async_tls1_3_test@retry_streaming",
    "h2_tls_certwatch_async_tls1_3_test@retry_streaming2",
    "h2_tls_certwatch_async_tls1_3_test@simple_delayed_request",
    "h2_tls_certwatch_sync_tls1_2_test@connectivity",
    "h2_tls_certwatch_sync_tls1_2_test@max_connection_idle",
    "h2_tls_certwatch_sync_tls1_2_test@retry_cancellation",
    "h2_tls_certwatch_sync_tls1_2_test@retry_server_pushback_delay",
    "h2_tls_certwatch_sync_tls1_2_test@simple_delayed_request",
    "h2_tls_certwatch_sync_tls1_2_test@write_buffering",
    "h2_tls_simple_test@bad_ping",
    "h2_tls_simple_test@connectivity",
    "h2_tls_simple_test@max_connection_idle",
    "h2_tls_simple_test@retry_per_attempt_recv_timeout",
    "h2_tls_simple_test@simple_delayed_request",
    "h2_tls_static_async_tls1_3_test@connectivity",
    "h2_tls_static_async_tls1_3_test@max_connection_idle",
    "h2_tls_static_async_tls1_3_test@retry_cancellation",
    "h2_tls_static_async_tls1_3_test@retry_per_attempt_recv_timeout",
    "h2_tls_static_async_tls1_3_test@simple_delayed_request",
    "h2_tls_test@bad_ping",
    "h2_tls_test@connectivity",
    "h2_tls_test@grpc_authz",
    "h2_tls_test@hpack_size",
    "h2_tls_test@invoke_large_request",
    "h2_tls_test@keepalive_timeout",
    "h2_tls_test@max_connection_idle",
    "h2_tls_test@resource_quota_server",
    "h2_tls_test@retry",
    "h2_tls_test@retry_cancellation",
    "h2_tls_test@retry_per_attempt_recv_timeout",
    "h2_tls_test@retry_streaming",
    "h2_tls_test@retry_streaming2",
    "h2_tls_test@simple_delayed_request",
    "h2_uds_test@bad_ping",
    "h2_uds_test@max_connection_idle",
    "h2_uds_test@retry_per_attempt_recv_timeout",
    "h2_uds_test@retry_streaming",
    "h2_uds_test@retry_transparent_mcs",
]
