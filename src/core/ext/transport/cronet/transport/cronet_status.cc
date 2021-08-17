/*
 *
 * Copyright 2020 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <grpc/support/port_platform.h>

#include "src/core/ext/transport/cronet/transport/cronet_status.h"

const char* cronet_net_error_as_string(cronet_net_error_code net_error) {
  switch (net_error) {
    case OK:
      return "OK";
    case CRONET_NET_ERROR_IO_PENDING:
      return "CRONET_NET_ERROR_IO_PENDING";
    case CRONET_NET_ERROR_FAILED:
      return "CRONET_NET_ERROR_FAILED";
    case CRONET_NET_ERROR_ABORTED:
      return "CRONET_NET_ERROR_ABORTED";
    case CRONET_NET_ERROR_INVALID_ARGUMENT:
      return "CRONET_NET_ERROR_INVALID_ARGUMENT";
    case CRONET_NET_ERROR_INVALID_HANDLE:
      return "CRONET_NET_ERROR_INVALID_HANDLE";
    case CRONET_NET_ERROR_FILE_NOT_FOUND:
      return "CRONET_NET_ERROR_FILE_NOT_FOUND";
    case CRONET_NET_ERROR_TIMED_OUT:
      return "CRONET_NET_ERROR_TIMED_OUT";
    case CRONET_NET_ERROR_FILE_TOO_BIG:
      return "CRONET_NET_ERROR_FILE_TOO_BIG";
    case CRONET_NET_ERROR_UNEXPECTED:
      return "CRONET_NET_ERROR_UNEXPECTED";
    case CRONET_NET_ERROR_ACCESS_DENIED:
      return "CRONET_NET_ERROR_ACCESS_DENIED";
    case CRONET_NET_ERROR_NOT_IMPLEMENTED:
      return "CRONET_NET_ERROR_NOT_IMPLEMENTED";
    case CRONET_NET_ERROR_INSUFFICIENT_RESOURCES:
      return "CRONET_NET_ERROR_INSUFFICIENT_RESOURCES";
    case CRONET_NET_ERROR_OUT_OF_MEMORY:
      return "CRONET_NET_ERROR_OUT_OF_MEMORY";
    case CRONET_NET_ERROR_UPLOAD_FILE_CHANGED:
      return "CRONET_NET_ERROR_UPLOAD_FILE_CHANGED";
    case CRONET_NET_ERROR_SOCKET_NOT_CONNECTED:
      return "CRONET_NET_ERROR_SOCKET_NOT_CONNECTED";
    case CRONET_NET_ERROR_FILE_EXISTS:
      return "CRONET_NET_ERROR_FILE_EXISTS";
    case CRONET_NET_ERROR_FILE_PATH_TOO_LONG:
      return "CRONET_NET_ERROR_FILE_PATH_TOO_LONG";
    case CRONET_NET_ERROR_FILE_NO_SPACE:
      return "CRONET_NET_ERROR_FILE_NO_SPACE";
    case CRONET_NET_ERROR_FILE_VIRUS_INFECTED:
      return "CRONET_NET_ERROR_FILE_VIRUS_INFECTED";
    case CRONET_NET_ERROR_BLOCKED_BY_CLIENT:
      return "CRONET_NET_ERROR_BLOCKED_BY_CLIENT";
    case CRONET_NET_ERROR_NETWORK_CHANGED:
      return "CRONET_NET_ERROR_NETWORK_CHANGED";
    case CRONET_NET_ERROR_BLOCKED_BY_ADMINISTRATOR:
      return "CRONET_NET_ERROR_BLOCKED_BY_ADMINISTRATOR";
    case CRONET_NET_ERROR_SOCKET_IS_CONNECTED:
      return "CRONET_NET_ERROR_SOCKET_IS_CONNECTED";
    case CRONET_NET_ERROR_BLOCKED_ENROLLMENT_CHECK_PENDING:
      return "CRONET_NET_ERROR_BLOCKED_ENROLLMENT_CHECK_PENDING";
    case CRONET_NET_ERROR_UPLOAD_STREAM_REWIND_NOT_SUPPORTED:
      return "CRONET_NET_ERROR_UPLOAD_STREAM_REWIND_NOT_SUPPORTED";
    case CRONET_NET_ERROR_CONTEXT_SHUT_DOWN:
      return "CRONET_NET_ERROR_CONTEXT_SHUT_DOWN";
    case CRONET_NET_ERROR_BLOCKED_BY_RESPONSE:
      return "CRONET_NET_ERROR_BLOCKED_BY_RESPONSE";
    case CRONET_NET_ERROR_CLEARTEXT_NOT_PERMITTED:
      return "CRONET_NET_ERROR_CLEARTEXT_NOT_PERMITTED";
    case CRONET_NET_ERROR_BLOCKED_BY_CSP:
      return "CRONET_NET_ERROR_BLOCKED_BY_CSP";
    case CRONET_NET_ERROR_H2_OR_QUIC_REQUIRED:
      return "CRONET_NET_ERROR_H2_OR_QUIC_REQUIRED";
    case CRONET_NET_ERROR_INSECURE_PRIVATE_NETWORK_REQUEST:
      return "CRONET_NET_ERROR_INSECURE_PRIVATE_NETWORK_REQUEST";
    case CRONET_NET_ERROR_CONNECTION_CLOSED:
      return "CRONET_NET_ERROR_CONNECTION_CLOSED";
    case CRONET_NET_ERROR_CONNECTION_RESET:
      return "CRONET_NET_ERROR_CONNECTION_RESET";
    case CRONET_NET_ERROR_CONNECTION_REFUSED:
      return "CRONET_NET_ERROR_CONNECTION_REFUSED";
    case CRONET_NET_ERROR_CONNECTION_ABORTED:
      return "CRONET_NET_ERROR_CONNECTION_ABORTED";
    case CRONET_NET_ERROR_CONNECTION_FAILED:
      return "CRONET_NET_ERROR_CONNECTION_FAILED";
    case CRONET_NET_ERROR_NAME_NOT_RESOLVED:
      return "CRONET_NET_ERROR_NAME_NOT_RESOLVED";
    case CRONET_NET_ERROR_INTERNET_DISCONNECTED:
      return "CRONET_NET_ERROR_INTERNET_DISCONNECTED";
    case CRONET_NET_ERROR_SSL_PROTOCOL_ERROR:
      return "CRONET_NET_ERROR_SSL_PROTOCOL_ERROR";
    case CRONET_NET_ERROR_ADDRESS_INVALID:
      return "CRONET_NET_ERROR_ADDRESS_INVALID";
    case CRONET_NET_ERROR_ADDRESS_UNREACHABLE:
      return "CRONET_NET_ERROR_ADDRESS_UNREACHABLE";
    case CRONET_NET_ERROR_SSL_CLIENT_AUTH_CERT_NEEDED:
      return "CRONET_NET_ERROR_SSL_CLIENT_AUTH_CERT_NEEDED";
    case CRONET_NET_ERROR_TUNNEL_CONNECTION_FAILED:
      return "CRONET_NET_ERROR_TUNNEL_CONNECTION_FAILED";
    case CRONET_NET_ERROR_NO_SSL_VERSIONS_ENABLED:
      return "CRONET_NET_ERROR_NO_SSL_VERSIONS_ENABLED";
    case CRONET_NET_ERROR_SSL_VERSION_OR_CIPHER_MISMATCH:
      return "CRONET_NET_ERROR_SSL_VERSION_OR_CIPHER_MISMATCH";
    case CRONET_NET_ERROR_SSL_RENEGOTIATION_REQUESTED:
      return "CRONET_NET_ERROR_SSL_RENEGOTIATION_REQUESTED";
    case CRONET_NET_ERROR_PROXY_AUTH_UNSUPPORTED:
      return "CRONET_NET_ERROR_PROXY_AUTH_UNSUPPORTED";
    case CRONET_NET_ERROR_CERT_ERROR_IN_SSL_RENEGOTIATION:
      return "CRONET_NET_ERROR_CERT_ERROR_IN_SSL_RENEGOTIATION";
    case CRONET_NET_ERROR_BAD_SSL_CLIENT_AUTH_CERT:
      return "CRONET_NET_ERROR_BAD_SSL_CLIENT_AUTH_CERT";
    case CRONET_NET_ERROR_CONNECTION_TIMED_OUT:
      return "CRONET_NET_ERROR_CONNECTION_TIMED_OUT";
    case CRONET_NET_ERROR_HOST_RESOLVER_QUEUE_TOO_LARGE:
      return "CRONET_NET_ERROR_HOST_RESOLVER_QUEUE_TOO_LARGE";
    case CRONET_NET_ERROR_SOCKS_CONNECTION_FAILED:
      return "CRONET_NET_ERROR_SOCKS_CONNECTION_FAILED";
    case CRONET_NET_ERROR_SOCKS_CONNECTION_HOST_UNREACHABLE:
      return "CRONET_NET_ERROR_SOCKS_CONNECTION_HOST_UNREACHABLE";
    case CRONET_NET_ERROR_ALPN_NEGOTIATION_FAILED:
      return "CRONET_NET_ERROR_ALPN_NEGOTIATION_FAILED";
    case CRONET_NET_ERROR_SSL_NO_RENEGOTIATION:
      return "CRONET_NET_ERROR_SSL_NO_RENEGOTIATION";
    case CRONET_NET_ERROR_WINSOCK_UNEXPECTED_WRITTEN_BYTES:
      return "CRONET_NET_ERROR_WINSOCK_UNEXPECTED_WRITTEN_BYTES";
    case CRONET_NET_ERROR_SSL_DECOMPRESSION_FAILURE_ALERT:
      return "CRONET_NET_ERROR_SSL_DECOMPRESSION_FAILURE_ALERT";
    case CRONET_NET_ERROR_SSL_BAD_RECORD_MAC_ALERT:
      return "CRONET_NET_ERROR_SSL_BAD_RECORD_MAC_ALERT";
    case CRONET_NET_ERROR_PROXY_AUTH_REQUESTED:
      return "CRONET_NET_ERROR_PROXY_AUTH_REQUESTED";
    case CRONET_NET_ERROR_PROXY_CONNECTION_FAILED:
      return "CRONET_NET_ERROR_PROXY_CONNECTION_FAILED";
    case CRONET_NET_ERROR_MANDATORY_PROXY_CONFIGURATION_FAILED:
      return "CRONET_NET_ERROR_MANDATORY_PROXY_CONFIGURATION_FAILED";
    case CRONET_NET_ERROR_PRECONNECT_MAX_SOCKET_LIMIT:
      return "CRONET_NET_ERROR_PRECONNECT_MAX_SOCKET_LIMIT";
    case CRONET_NET_ERROR_SSL_CLIENT_AUTH_PRIVATE_KEY_ACCESS_DENIED:
      return "CRONET_NET_ERROR_SSL_CLIENT_AUTH_PRIVATE_KEY_ACCESS_DENIED";
    case CRONET_NET_ERROR_SSL_CLIENT_AUTH_CERT_NO_PRIVATE_KEY:
      return "CRONET_NET_ERROR_SSL_CLIENT_AUTH_CERT_NO_PRIVATE_KEY";
    case CRONET_NET_ERROR_PROXY_CERTIFICATE_INVALID:
      return "CRONET_NET_ERROR_PROXY_CERTIFICATE_INVALID";
    case CRONET_NET_ERROR_NAME_RESOLUTION_FAILED:
      return "CRONET_NET_ERROR_NAME_RESOLUTION_FAILED";
    case CRONET_NET_ERROR_NETWORK_ACCESS_DENIED:
      return "CRONET_NET_ERROR_NETWORK_ACCESS_DENIED";
    case CRONET_NET_ERROR_TEMPORARILY_THROTTLED:
      return "CRONET_NET_ERROR_TEMPORARILY_THROTTLED";
    case CRONET_NET_ERROR_HTTPS_PROXY_TUNNEL_RESPONSE_REDIRECT:
      return "CRONET_NET_ERROR_HTTPS_PROXY_TUNNEL_RESPONSE_REDIRECT";
    case CRONET_NET_ERROR_SSL_CLIENT_AUTH_SIGNATURE_FAILED:
      return "CRONET_NET_ERROR_SSL_CLIENT_AUTH_SIGNATURE_FAILED";
    case CRONET_NET_ERROR_MSG_TOO_BIG:
      return "CRONET_NET_ERROR_MSG_TOO_BIG";
    case CRONET_NET_ERROR_WS_PROTOCOL_ERROR:
      return "CRONET_NET_ERROR_WS_PROTOCOL_ERROR";
    case CRONET_NET_ERROR_ADDRESS_IN_USE:
      return "CRONET_NET_ERROR_ADDRESS_IN_USE";
    case CRONET_NET_ERROR_SSL_HANDSHAKE_NOT_COMPLETED:
      return "CRONET_NET_ERROR_SSL_HANDSHAKE_NOT_COMPLETED";
    case CRONET_NET_ERROR_SSL_BAD_PEER_PUBLIC_KEY:
      return "CRONET_NET_ERROR_SSL_BAD_PEER_PUBLIC_KEY";
    case CRONET_NET_ERROR_SSL_PINNED_KEY_NOT_IN_CERT_CHAIN:
      return "CRONET_NET_ERROR_SSL_PINNED_KEY_NOT_IN_CERT_CHAIN";
    case CRONET_NET_ERROR_CLIENT_AUTH_CERT_TYPE_UNSUPPORTED:
      return "CRONET_NET_ERROR_CLIENT_AUTH_CERT_TYPE_UNSUPPORTED";
    case CRONET_NET_ERROR_SSL_DECRYPT_ERROR_ALERT:
      return "CRONET_NET_ERROR_SSL_DECRYPT_ERROR_ALERT";
    case CRONET_NET_ERROR_WS_THROTTLE_QUEUE_TOO_LARGE:
      return "CRONET_NET_ERROR_WS_THROTTLE_QUEUE_TOO_LARGE";
    case CRONET_NET_ERROR_SSL_SERVER_CERT_CHANGED:
      return "CRONET_NET_ERROR_SSL_SERVER_CERT_CHANGED";
    case CRONET_NET_ERROR_SSL_UNRECOGNIZED_NAME_ALERT:
      return "CRONET_NET_ERROR_SSL_UNRECOGNIZED_NAME_ALERT";
    case CRONET_NET_ERROR_SOCKET_SET_RECEIVE_BUFFER_SIZE_ERROR:
      return "CRONET_NET_ERROR_SOCKET_SET_RECEIVE_BUFFER_SIZE_ERROR";
    case CRONET_NET_ERROR_SOCKET_SET_SEND_BUFFER_SIZE_ERROR:
      return "CRONET_NET_ERROR_SOCKET_SET_SEND_BUFFER_SIZE_ERROR";
    case CRONET_NET_ERROR_SOCKET_RECEIVE_BUFFER_SIZE_UNCHANGEABLE:
      return "CRONET_NET_ERROR_SOCKET_RECEIVE_BUFFER_SIZE_UNCHANGEABLE";
    case CRONET_NET_ERROR_SOCKET_SEND_BUFFER_SIZE_UNCHANGEABLE:
      return "CRONET_NET_ERROR_SOCKET_SEND_BUFFER_SIZE_UNCHANGEABLE";
    case CRONET_NET_ERROR_SSL_CLIENT_AUTH_CERT_BAD_FORMAT:
      return "CRONET_NET_ERROR_SSL_CLIENT_AUTH_CERT_BAD_FORMAT";
    case CRONET_NET_ERROR_ICANN_NAME_COLLISION:
      return "CRONET_NET_ERROR_ICANN_NAME_COLLISION";
    case CRONET_NET_ERROR_SSL_SERVER_CERT_BAD_FORMAT:
      return "CRONET_NET_ERROR_SSL_SERVER_CERT_BAD_FORMAT";
    case CRONET_NET_ERROR_CT_STH_PARSING_FAILED:
      return "CRONET_NET_ERROR_CT_STH_PARSING_FAILED";
    case CRONET_NET_ERROR_CT_STH_INCOMPLETE:
      return "CRONET_NET_ERROR_CT_STH_INCOMPLETE";
    case CRONET_NET_ERROR_UNABLE_TO_REUSE_CONNECTION_FOR_PROXY_AUTH:
      return "CRONET_NET_ERROR_UNABLE_TO_REUSE_CONNECTION_FOR_PROXY_AUTH";
    case CRONET_NET_ERROR_CT_CONSISTENCY_PROOF_PARSING_FAILED:
      return "CRONET_NET_ERROR_CT_CONSISTENCY_PROOF_PARSING_FAILED";
    case CRONET_NET_ERROR_SSL_OBSOLETE_CIPHER:
      return "CRONET_NET_ERROR_SSL_OBSOLETE_CIPHER";
    case CRONET_NET_ERROR_WS_UPGRADE:
      return "CRONET_NET_ERROR_WS_UPGRADE";
    case CRONET_NET_ERROR_READ_IF_READY_NOT_IMPLEMENTED:
      return "CRONET_NET_ERROR_READ_IF_READY_NOT_IMPLEMENTED";
    case CRONET_NET_ERROR_NO_BUFFER_SPACE:
      return "CRONET_NET_ERROR_NO_BUFFER_SPACE";
    case CRONET_NET_ERROR_SSL_CLIENT_AUTH_NO_COMMON_ALGORITHMS:
      return "CRONET_NET_ERROR_SSL_CLIENT_AUTH_NO_COMMON_ALGORITHMS";
    case CRONET_NET_ERROR_EARLY_DATA_REJECTED:
      return "CRONET_NET_ERROR_EARLY_DATA_REJECTED";
    case CRONET_NET_ERROR_WRONG_VERSION_ON_EARLY_DATA:
      return "CRONET_NET_ERROR_WRONG_VERSION_ON_EARLY_DATA";
    case CRONET_NET_ERROR_TLS13_DOWNGRADE_DETECTED:
      return "CRONET_NET_ERROR_TLS13_DOWNGRADE_DETECTED";
    case CRONET_NET_ERROR_SSL_KEY_USAGE_INCOMPATIBLE:
      return "CRONET_NET_ERROR_SSL_KEY_USAGE_INCOMPATIBLE";
    case CRONET_NET_ERROR_CERT_COMMON_NAME_INVALID:
      return "CRONET_NET_ERROR_CERT_COMMON_NAME_INVALID";
    case CRONET_NET_ERROR_CERT_DATE_INVALID:
      return "CRONET_NET_ERROR_CERT_DATE_INVALID";
    case CRONET_NET_ERROR_CERT_AUTHORITY_INVALID:
      return "CRONET_NET_ERROR_CERT_AUTHORITY_INVALID";
    case CRONET_NET_ERROR_CERT_CONTAINS_ERRORS:
      return "CRONET_NET_ERROR_CERT_CONTAINS_ERRORS";
    case CRONET_NET_ERROR_CERT_NO_REVOCATION_MECHANISM:
      return "CRONET_NET_ERROR_CERT_NO_REVOCATION_MECHANISM";
    case CRONET_NET_ERROR_CERT_UNABLE_TO_CHECK_REVOCATION:
      return "CRONET_NET_ERROR_CERT_UNABLE_TO_CHECK_REVOCATION";
    case CRONET_NET_ERROR_CERT_REVOKED:
      return "CRONET_NET_ERROR_CERT_REVOKED";
    case CRONET_NET_ERROR_CERT_INVALID:
      return "CRONET_NET_ERROR_CERT_INVALID";
    case CRONET_NET_ERROR_CERT_WEAK_SIGNATURE_ALGORITHM:
      return "CRONET_NET_ERROR_CERT_WEAK_SIGNATURE_ALGORITHM";
    case CRONET_NET_ERROR_CERT_NON_UNIQUE_NAME:
      return "CRONET_NET_ERROR_CERT_NON_UNIQUE_NAME";
    case CRONET_NET_ERROR_CERT_WEAK_KEY:
      return "CRONET_NET_ERROR_CERT_WEAK_KEY";
    case CRONET_NET_ERROR_CERT_NAME_CONSTRAINT_VIOLATION:
      return "CRONET_NET_ERROR_CERT_NAME_CONSTRAINT_VIOLATION";
    case CRONET_NET_ERROR_CERT_VALIDITY_TOO_LONG:
      return "CRONET_NET_ERROR_CERT_VALIDITY_TOO_LONG";
    case CRONET_NET_ERROR_CERTIFICATE_TRANSPARENCY_REQUIRED:
      return "CRONET_NET_ERROR_CERTIFICATE_TRANSPARENCY_REQUIRED";
    case CRONET_NET_ERROR_CERT_SYMANTEC_LEGACY:
      return "CRONET_NET_ERROR_CERT_SYMANTEC_LEGACY";
    case CRONET_NET_ERROR_CERT_KNOWN_INTERCEPTION_BLOCKED:
      return "CRONET_NET_ERROR_CERT_KNOWN_INTERCEPTION_BLOCKED";
    case CRONET_NET_ERROR_SSL_OBSOLETE_VERSION:
      return "CRONET_NET_ERROR_SSL_OBSOLETE_VERSION";
    case CRONET_NET_ERROR_CERT_END:
      return "CRONET_NET_ERROR_CERT_END";
    case CRONET_NET_ERROR_INVALID_URL:
      return "CRONET_NET_ERROR_INVALID_URL";
    case CRONET_NET_ERROR_DISALLOWED_URL_SCHEME:
      return "CRONET_NET_ERROR_DISALLOWED_URL_SCHEME";
    case CRONET_NET_ERROR_UNKNOWN_URL_SCHEME:
      return "CRONET_NET_ERROR_UNKNOWN_URL_SCHEME";
    case CRONET_NET_ERROR_INVALID_REDIRECT:
      return "CRONET_NET_ERROR_INVALID_REDIRECT";
    case CRONET_NET_ERROR_TOO_MANY_REDIRECTS:
      return "CRONET_NET_ERROR_TOO_MANY_REDIRECTS";
    case CRONET_NET_ERROR_UNSAFE_REDIRECT:
      return "CRONET_NET_ERROR_UNSAFE_REDIRECT";
    case CRONET_NET_ERROR_UNSAFE_PORT:
      return "CRONET_NET_ERROR_UNSAFE_PORT";
    case CRONET_NET_ERROR_INVALID_RESPONSE:
      return "CRONET_NET_ERROR_INVALID_RESPONSE";
    case CRONET_NET_ERROR_INVALID_CHUNKED_ENCODING:
      return "CRONET_NET_ERROR_INVALID_CHUNKED_ENCODING";
    case CRONET_NET_ERROR_METHOD_NOT_SUPPORTED:
      return "CRONET_NET_ERROR_METHOD_NOT_SUPPORTED";
    case CRONET_NET_ERROR_UNEXPECTED_PROXY_AUTH:
      return "CRONET_NET_ERROR_UNEXPECTED_PROXY_AUTH";
    case CRONET_NET_ERROR_EMPTY_RESPONSE:
      return "CRONET_NET_ERROR_EMPTY_RESPONSE";
    case CRONET_NET_ERROR_RESPONSE_HEADERS_TOO_BIG:
      return "CRONET_NET_ERROR_RESPONSE_HEADERS_TOO_BIG";
    case CRONET_NET_ERROR_PAC_SCRIPT_FAILED:
      return "CRONET_NET_ERROR_PAC_SCRIPT_FAILED";
    case CRONET_NET_ERROR_REQUEST_RANGE_NOT_SATISFIABLE:
      return "CRONET_NET_ERROR_REQUEST_RANGE_NOT_SATISFIABLE";
    case CRONET_NET_ERROR_MALFORMED_IDENTITY:
      return "CRONET_NET_ERROR_MALFORMED_IDENTITY";
    case CRONET_NET_ERROR_CONTENT_DECODING_FAILED:
      return "CRONET_NET_ERROR_CONTENT_DECODING_FAILED";
    case CRONET_NET_ERROR_NETWORK_IO_SUSPENDED:
      return "CRONET_NET_ERROR_NETWORK_IO_SUSPENDED";
    case CRONET_NET_ERROR_SYN_REPLY_NOT_RECEIVED:
      return "CRONET_NET_ERROR_SYN_REPLY_NOT_RECEIVED";
    case CRONET_NET_ERROR_ENCODING_CONVERSION_FAILED:
      return "CRONET_NET_ERROR_ENCODING_CONVERSION_FAILED";
    case CRONET_NET_ERROR_UNRECOGNIZED_FTP_DIRECTORY_LISTING_FORMAT:
      return "CRONET_NET_ERROR_UNRECOGNIZED_FTP_DIRECTORY_LISTING_FORMAT";
      return "CRONET_NET_ERROR_INVALID_SPDY_STREAM";
    case CRONET_NET_ERROR_NO_SUPPORTED_PROXIES:
      return "CRONET_NET_ERROR_NO_SUPPORTED_PROXIES";
    case CRONET_NET_ERROR_HTTP2_PROTOCOL_ERROR:
      return "CRONET_NET_ERROR_HTTP2_PROTOCOL_ERROR";
    case CRONET_NET_ERROR_INVALID_AUTH_CREDENTIALS:
      return "CRONET_NET_ERROR_INVALID_AUTH_CREDENTIALS";
    case CRONET_NET_ERROR_UNSUPPORTED_AUTH_SCHEME:
      return "CRONET_NET_ERROR_UNSUPPORTED_AUTH_SCHEME";
    case CRONET_NET_ERROR_ENCODING_DETECTION_FAILED:
      return "CRONET_NET_ERROR_ENCODING_DETECTION_FAILED";
    case CRONET_NET_ERROR_MISSING_AUTH_CREDENTIALS:
      return "CRONET_NET_ERROR_MISSING_AUTH_CREDENTIALS";
    case CRONET_NET_ERROR_UNEXPECTED_SECURITY_LIBRARY_STATUS:
      return "CRONET_NET_ERROR_UNEXPECTED_SECURITY_LIBRARY_STATUS";
    case CRONET_NET_ERROR_MISCONFIGURED_AUTH_ENVIRONMENT:
      return "CRONET_NET_ERROR_MISCONFIGURED_AUTH_ENVIRONMENT";
    case CRONET_NET_ERROR_UNDOCUMENTED_SECURITY_LIBRARY_STATUS:
      return "CRONET_NET_ERROR_UNDOCUMENTED_SECURITY_LIBRARY_STATUS";
    case CRONET_NET_ERROR_RESPONSE_BODY_TOO_BIG_TO_DRAIN:
      return "CRONET_NET_ERROR_RESPONSE_BODY_TOO_BIG_TO_DRAIN";
    case CRONET_NET_ERROR_RESPONSE_HEADERS_MULTIPLE_CONTENT_LENGTH:
      return "CRONET_NET_ERROR_RESPONSE_HEADERS_MULTIPLE_CONTENT_LENGTH";
    case CRONET_NET_ERROR_INCOMPLETE_HTTP2_HEADERS:
      return "CRONET_NET_ERROR_INCOMPLETE_HTTP2_HEADERS";
    case CRONET_NET_ERROR_PAC_NOT_IN_DHCP:
      return "CRONET_NET_ERROR_PAC_NOT_IN_DHCP";
    case CRONET_NET_ERROR_RESPONSE_HEADERS_MULTIPLE_CONTENT_DISPOSITION:
      return "CRONET_NET_ERROR_RESPONSE_HEADERS_MULTIPLE_CONTENT_DISPOSITION";
    case CRONET_NET_ERROR_RESPONSE_HEADERS_MULTIPLE_LOCATION:
      return "CRONET_NET_ERROR_RESPONSE_HEADERS_MULTIPLE_LOCATION";
    case CRONET_NET_ERROR_HTTP2_SERVER_REFUSED_STREAM:
      return "CRONET_NET_ERROR_HTTP2_SERVER_REFUSED_STREAM";
    case CRONET_NET_ERROR_HTTP2_PING_FAILED:
      return "CRONET_NET_ERROR_HTTP2_PING_FAILED";
      return "CRONET_NET_ERROR_PIPELINE_EVICTION";
    case CRONET_NET_ERROR_CONTENT_LENGTH_MISMATCH:
      return "CRONET_NET_ERROR_CONTENT_LENGTH_MISMATCH";
    case CRONET_NET_ERROR_INCOMPLETE_CHUNKED_ENCODING:
      return "CRONET_NET_ERROR_INCOMPLETE_CHUNKED_ENCODING";
    case CRONET_NET_ERROR_QUIC_PROTOCOL_ERROR:
      return "CRONET_NET_ERROR_QUIC_PROTOCOL_ERROR";
    case CRONET_NET_ERROR_RESPONSE_HEADERS_TRUNCATED:
      return "CRONET_NET_ERROR_RESPONSE_HEADERS_TRUNCATED";
    case CRONET_NET_ERROR_QUIC_HANDSHAKE_FAILED:
      return "CRONET_NET_ERROR_QUIC_HANDSHAKE_FAILED";
      return "CRONET_NET_ERROR_REQUEST_FOR_SECURE_RESOURCE_OVER_INSECURE_QUIC";
    case CRONET_NET_ERROR_HTTP2_INADEQUATE_TRANSPORT_SECURITY:
      return "CRONET_NET_ERROR_HTTP2_INADEQUATE_TRANSPORT_SECURITY";
    case CRONET_NET_ERROR_HTTP2_FLOW_CONTROL_ERROR:
      return "CRONET_NET_ERROR_HTTP2_FLOW_CONTROL_ERROR";
    case CRONET_NET_ERROR_HTTP2_FRAME_SIZE_ERROR:
      return "CRONET_NET_ERROR_HTTP2_FRAME_SIZE_ERROR";
    case CRONET_NET_ERROR_HTTP2_COMPRESSION_ERROR:
      return "CRONET_NET_ERROR_HTTP2_COMPRESSION_ERROR";
    case CRONET_NET_ERROR_PROXY_AUTH_REQUESTED_WITH_NO_CONNECTION:
      return "CRONET_NET_ERROR_PROXY_AUTH_REQUESTED_WITH_NO_CONNECTION";
    case CRONET_NET_ERROR_HTTP_1_1_REQUIRED:
      return "CRONET_NET_ERROR_HTTP_1_1_REQUIRED";
    case CRONET_NET_ERROR_PROXY_HTTP_1_1_REQUIRED:
      return "CRONET_NET_ERROR_PROXY_HTTP_1_1_REQUIRED";
    case CRONET_NET_ERROR_PAC_SCRIPT_TERMINATED:
      return "CRONET_NET_ERROR_PAC_SCRIPT_TERMINATED";
      return "CRONET_NET_ERROR_TEMPORARY_BACKOFF";
    case CRONET_NET_ERROR_INVALID_HTTP_RESPONSE:
      return "CRONET_NET_ERROR_INVALID_HTTP_RESPONSE";
    case CRONET_NET_ERROR_CONTENT_DECODING_INIT_FAILED:
      return "CRONET_NET_ERROR_CONTENT_DECODING_INIT_FAILED";
    case CRONET_NET_ERROR_HTTP2_RST_STREAM_NO_ERROR_RECEIVED:
      return "CRONET_NET_ERROR_HTTP2_RST_STREAM_NO_ERROR_RECEIVED";
    case CRONET_NET_ERROR_HTTP2_PUSHED_STREAM_NOT_AVAILABLE:
      return "CRONET_NET_ERROR_HTTP2_PUSHED_STREAM_NOT_AVAILABLE";
    case CRONET_NET_ERROR_HTTP2_CLAIMED_PUSHED_STREAM_RESET_BY_SERVER:
      return "CRONET_NET_ERROR_HTTP2_CLAIMED_PUSHED_STREAM_RESET_BY_SERVER";
    case CRONET_NET_ERROR_TOO_MANY_RETRIES:
      return "CRONET_NET_ERROR_TOO_MANY_RETRIES";
    case CRONET_NET_ERROR_HTTP2_STREAM_CLOSED:
      return "CRONET_NET_ERROR_HTTP2_STREAM_CLOSED";
    case CRONET_NET_ERROR_HTTP2_CLIENT_REFUSED_STREAM:
      return "CRONET_NET_ERROR_HTTP2_CLIENT_REFUSED_STREAM";
    case CRONET_NET_ERROR_HTTP2_PUSHED_RESPONSE_DOES_NOT_MATCH:
      return "CRONET_NET_ERROR_HTTP2_PUSHED_RESPONSE_DOES_NOT_MATCH";
    case CRONET_NET_ERROR_HTTP_RESPONSE_CODE_FAILURE:
      return "CRONET_NET_ERROR_HTTP_RESPONSE_CODE_FAILURE";
    case CRONET_NET_ERROR_QUIC_CERT_ROOT_NOT_KNOWN:
      return "CRONET_NET_ERROR_QUIC_CERT_ROOT_NOT_KNOWN";
    case CRONET_NET_ERROR_CACHE_MISS:
      return "CRONET_NET_ERROR_CACHE_MISS";
    case CRONET_NET_ERROR_CACHE_READ_FAILURE:
      return "CRONET_NET_ERROR_CACHE_READ_FAILURE";
    case CRONET_NET_ERROR_CACHE_WRITE_FAILURE:
      return "CRONET_NET_ERROR_CACHE_WRITE_FAILURE";
    case CRONET_NET_ERROR_CACHE_OPERATION_NOT_SUPPORTED:
      return "CRONET_NET_ERROR_CACHE_OPERATION_NOT_SUPPORTED";
    case CRONET_NET_ERROR_CACHE_OPEN_FAILURE:
      return "CRONET_NET_ERROR_CACHE_OPEN_FAILURE";
    case CRONET_NET_ERROR_CACHE_CREATE_FAILURE:
      return "CRONET_NET_ERROR_CACHE_CREATE_FAILURE";
    case CRONET_NET_ERROR_CACHE_RACE:
      return "CRONET_NET_ERROR_CACHE_RACE";
    case CRONET_NET_ERROR_CACHE_CHECKSUM_READ_FAILURE:
      return "CRONET_NET_ERROR_CACHE_CHECKSUM_READ_FAILURE";
    case CRONET_NET_ERROR_CACHE_CHECKSUM_MISMATCH:
      return "CRONET_NET_ERROR_CACHE_CHECKSUM_MISMATCH";
    case CRONET_NET_ERROR_CACHE_LOCK_TIMEOUT:
      return "CRONET_NET_ERROR_CACHE_LOCK_TIMEOUT";
    case CRONET_NET_ERROR_CACHE_AUTH_FAILURE_AFTER_READ:
      return "CRONET_NET_ERROR_CACHE_AUTH_FAILURE_AFTER_READ";
    case CRONET_NET_ERROR_CACHE_ENTRY_NOT_SUITABLE:
      return "CRONET_NET_ERROR_CACHE_ENTRY_NOT_SUITABLE";
    case CRONET_NET_ERROR_CACHE_DOOM_FAILURE:
      return "CRONET_NET_ERROR_CACHE_DOOM_FAILURE";
    case CRONET_NET_ERROR_CACHE_OPEN_OR_CREATE_FAILURE:
      return "CRONET_NET_ERROR_CACHE_OPEN_OR_CREATE_FAILURE";
    case CRONET_NET_ERROR_INSECURE_RESPONSE:
      return "CRONET_NET_ERROR_INSECURE_RESPONSE";
    case CRONET_NET_ERROR_NO_PRIVATE_KEY_FOR_CERT:
      return "CRONET_NET_ERROR_NO_PRIVATE_KEY_FOR_CERT";
    case CRONET_NET_ERROR_ADD_USER_CERT_FAILED:
      return "CRONET_NET_ERROR_ADD_USER_CERT_FAILED";
    case CRONET_NET_ERROR_INVALID_SIGNED_EXCHANGE:
      return "CRONET_NET_ERROR_INVALID_SIGNED_EXCHANGE";
    case CRONET_NET_ERROR_INVALID_WEB_BUNDLE:
      return "CRONET_NET_ERROR_INVALID_WEB_BUNDLE";
    case CRONET_NET_ERROR_TRUST_TOKEN_OPERATION_FAILED:
      return "CRONET_NET_ERROR_TRUST_TOKEN_OPERATION_FAILED";
    case CRONET_NET_ERROR_TRUST_TOKEN_OPERATION_CACHE_HIT:
      return "CRONET_NET_ERROR_TRUST_TOKEN_OPERATION_CACHE_HIT";
    case CRONET_NET_ERROR_FTP_FAILED:
      return "CRONET_NET_ERROR_FTP_FAILED";
    case CRONET_NET_ERROR_FTP_SERVICE_UNAVAILABLE:
      return "CRONET_NET_ERROR_FTP_SERVICE_UNAVAILABLE";
    case CRONET_NET_ERROR_FTP_TRANSFER_ABORTED:
      return "CRONET_NET_ERROR_FTP_TRANSFER_ABORTED";
    case CRONET_NET_ERROR_FTP_FILE_BUSY:
      return "CRONET_NET_ERROR_FTP_FILE_BUSY";
    case CRONET_NET_ERROR_FTP_SYNTAX_ERROR:
      return "CRONET_NET_ERROR_FTP_SYNTAX_ERROR";
    case CRONET_NET_ERROR_FTP_COMMAND_NOT_SUPPORTED:
      return "CRONET_NET_ERROR_FTP_COMMAND_NOT_SUPPORTED";
    case CRONET_NET_ERROR_FTP_BAD_COMMAND_SEQUENCE:
      return "CRONET_NET_ERROR_FTP_BAD_COMMAND_SEQUENCE";
    case CRONET_NET_ERROR_PKCS12_IMPORT_BAD_PASSWORD:
      return "CRONET_NET_ERROR_PKCS12_IMPORT_BAD_PASSWORD";
    case CRONET_NET_ERROR_PKCS12_IMPORT_FAILED:
      return "CRONET_NET_ERROR_PKCS12_IMPORT_FAILED";
    case CRONET_NET_ERROR_IMPORT_CA_CERT_NOT_CA:
      return "CRONET_NET_ERROR_IMPORT_CA_CERT_NOT_CA";
    case CRONET_NET_ERROR_IMPORT_CERT_ALREADY_EXISTS:
      return "CRONET_NET_ERROR_IMPORT_CERT_ALREADY_EXISTS";
    case CRONET_NET_ERROR_IMPORT_CA_CERT_FAILED:
      return "CRONET_NET_ERROR_IMPORT_CA_CERT_FAILED";
    case CRONET_NET_ERROR_IMPORT_SERVER_CERT_FAILED:
      return "CRONET_NET_ERROR_IMPORT_SERVER_CERT_FAILED";
    case CRONET_NET_ERROR_PKCS12_IMPORT_INVALID_MAC:
      return "CRONET_NET_ERROR_PKCS12_IMPORT_INVALID_MAC";
    case CRONET_NET_ERROR_PKCS12_IMPORT_INVALID_FILE:
      return "CRONET_NET_ERROR_PKCS12_IMPORT_INVALID_FILE";
    case CRONET_NET_ERROR_PKCS12_IMPORT_UNSUPPORTED:
      return "CRONET_NET_ERROR_PKCS12_IMPORT_UNSUPPORTED";
    case CRONET_NET_ERROR_KEY_GENERATION_FAILED:
      return "CRONET_NET_ERROR_KEY_GENERATION_FAILED";
    case CRONET_NET_ERROR_PRIVATE_KEY_EXPORT_FAILED:
      return "CRONET_NET_ERROR_PRIVATE_KEY_EXPORT_FAILED";
    case CRONET_NET_ERROR_SELF_SIGNED_CERT_GENERATION_FAILED:
      return "CRONET_NET_ERROR_SELF_SIGNED_CERT_GENERATION_FAILED";
    case CRONET_NET_ERROR_CERT_DATABASE_CHANGED:
      return "CRONET_NET_ERROR_CERT_DATABASE_CHANGED";
    case CRONET_NET_ERROR_DNS_MALFORMED_RESPONSE:
      return "CRONET_NET_ERROR_DNS_MALFORMED_RESPONSE";
    case CRONET_NET_ERROR_DNS_SERVER_REQUIRES_TCP:
      return "CRONET_NET_ERROR_DNS_SERVER_REQUIRES_TCP";
    case CRONET_NET_ERROR_DNS_SERVER_FAILED:
      return "CRONET_NET_ERROR_DNS_SERVER_FAILED";
    case CRONET_NET_ERROR_DNS_TIMED_OUT:
      return "CRONET_NET_ERROR_DNS_TIMED_OUT";
    case CRONET_NET_ERROR_DNS_CACHE_MISS:
      return "CRONET_NET_ERROR_DNS_CACHE_MISS";
    case CRONET_NET_ERROR_DNS_SEARCH_EMPTY:
      return "CRONET_NET_ERROR_DNS_SEARCH_EMPTY";
    case CRONET_NET_ERROR_DNS_SORT_ERROR:
      return "CRONET_NET_ERROR_DNS_SORT_ERROR";
    case CRONET_NET_ERROR_DNS_SECURE_RESOLVER_HOSTNAME_RESOLUTION_FAILED:
      return "CRONET_NET_ERROR_DNS_SECURE_RESOLVER_HOSTNAME_RESOLUTION_FAILED";
  }
  return "UNAVAILABLE.";
}

grpc_status_code cronet_net_error_to_grpc_error(
    cronet_net_error_code net_error) {
  switch (net_error) {
    case OK:
      return GRPC_STATUS_OK;
    case CRONET_NET_ERROR_ABORTED:
      return GRPC_STATUS_ABORTED;
    case CRONET_NET_ERROR_ACCESS_DENIED:
    case CRONET_NET_ERROR_NETWORK_ACCESS_DENIED:
      return GRPC_STATUS_PERMISSION_DENIED;
    case CRONET_NET_ERROR_SSL_CLIENT_AUTH_CERT_NEEDED:
    case CRONET_NET_ERROR_PROXY_AUTH_UNSUPPORTED:
    case CRONET_NET_ERROR_BAD_SSL_CLIENT_AUTH_CERT:
    case CRONET_NET_ERROR_PROXY_AUTH_REQUESTED:
    case CRONET_NET_ERROR_SSL_CLIENT_AUTH_PRIVATE_KEY_ACCESS_DENIED:
    case CRONET_NET_ERROR_SSL_CLIENT_AUTH_CERT_NO_PRIVATE_KEY:
    case CRONET_NET_ERROR_SSL_CLIENT_AUTH_SIGNATURE_FAILED:
    case CRONET_NET_ERROR_CLIENT_AUTH_CERT_TYPE_UNSUPPORTED:
    case CRONET_NET_ERROR_SSL_CLIENT_AUTH_CERT_BAD_FORMAT:
    case CRONET_NET_ERROR_SSL_CLIENT_AUTH_NO_COMMON_ALGORITHMS:
    case CRONET_NET_ERROR_CERT_AUTHORITY_INVALID:
    case CRONET_NET_ERROR_UNEXPECTED_PROXY_AUTH:
    case CRONET_NET_ERROR_MALFORMED_IDENTITY:
    case CRONET_NET_ERROR_INVALID_AUTH_CREDENTIALS:
    case CRONET_NET_ERROR_UNSUPPORTED_AUTH_SCHEME:
    case CRONET_NET_ERROR_MISSING_AUTH_CREDENTIALS:
      return GRPC_STATUS_UNAUTHENTICATED;
    default:
      return GRPC_STATUS_UNAVAILABLE;
  }

  return GRPC_STATUS_UNAVAILABLE;
}
