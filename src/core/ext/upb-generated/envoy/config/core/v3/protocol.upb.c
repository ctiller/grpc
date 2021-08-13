/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/config/core/v3/protocol.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include <stddef.h>
#include "upb/msg_internal.h"
#include "envoy/config/core/v3/protocol.upb.h"
#include "envoy/config/core/v3/extension.upb.h"
#include "envoy/type/v3/percent.upb.h"
#include "google/protobuf/duration.upb.h"
#include "google/protobuf/wrappers.upb.h"
#include "envoy/annotations/deprecation.upb.h"
#include "udpa/annotations/status.upb.h"
#include "udpa/annotations/versioning.upb.h"
#include "validate/validate.upb.h"

#include "upb/port_def.inc"

const upb_msglayout envoy_config_core_v3_TcpProtocolOptions_msginit = {
  NULL,
  NULL,
  UPB_SIZE(0, 0), 0, false, 0, 255,
};

static const upb_msglayout *const envoy_config_core_v3_QuicProtocolOptions_submsgs[1] = {
  &google_protobuf_UInt32Value_msginit,
};

static const upb_msglayout_field envoy_config_core_v3_QuicProtocolOptions__fields[3] = {
  {1, UPB_SIZE(4, 8), 1, 0, 11, _UPB_MODE_SCALAR},
  {2, UPB_SIZE(8, 16), 2, 0, 11, _UPB_MODE_SCALAR},
  {3, UPB_SIZE(12, 24), 3, 0, 11, _UPB_MODE_SCALAR},
};

const upb_msglayout envoy_config_core_v3_QuicProtocolOptions_msginit = {
  &envoy_config_core_v3_QuicProtocolOptions_submsgs[0],
  &envoy_config_core_v3_QuicProtocolOptions__fields[0],
  UPB_SIZE(16, 32), 3, false, 3, 255,
};

static const upb_msglayout_field envoy_config_core_v3_UpstreamHttpProtocolOptions__fields[2] = {
  {1, UPB_SIZE(0, 0), 0, 0, 8, _UPB_MODE_SCALAR},
  {2, UPB_SIZE(1, 1), 0, 0, 8, _UPB_MODE_SCALAR},
};

const upb_msglayout envoy_config_core_v3_UpstreamHttpProtocolOptions_msginit = {
  NULL,
  &envoy_config_core_v3_UpstreamHttpProtocolOptions__fields[0],
  UPB_SIZE(8, 8), 2, false, 2, 255,
};

static const upb_msglayout *const envoy_config_core_v3_AlternateProtocolsCacheOptions_submsgs[1] = {
  &google_protobuf_UInt32Value_msginit,
};

static const upb_msglayout_field envoy_config_core_v3_AlternateProtocolsCacheOptions__fields[2] = {
  {1, UPB_SIZE(4, 8), 0, 0, 9, _UPB_MODE_SCALAR},
  {2, UPB_SIZE(12, 24), 1, 0, 11, _UPB_MODE_SCALAR},
};

const upb_msglayout envoy_config_core_v3_AlternateProtocolsCacheOptions_msginit = {
  &envoy_config_core_v3_AlternateProtocolsCacheOptions_submsgs[0],
  &envoy_config_core_v3_AlternateProtocolsCacheOptions__fields[0],
  UPB_SIZE(16, 32), 2, false, 2, 255,
};

static const upb_msglayout *const envoy_config_core_v3_HttpProtocolOptions_submsgs[2] = {
  &google_protobuf_Duration_msginit,
  &google_protobuf_UInt32Value_msginit,
};

static const upb_msglayout_field envoy_config_core_v3_HttpProtocolOptions__fields[6] = {
  {1, UPB_SIZE(8, 8), 1, 0, 11, _UPB_MODE_SCALAR},
  {2, UPB_SIZE(12, 16), 2, 1, 11, _UPB_MODE_SCALAR},
  {3, UPB_SIZE(16, 24), 3, 0, 11, _UPB_MODE_SCALAR},
  {4, UPB_SIZE(20, 32), 4, 0, 11, _UPB_MODE_SCALAR},
  {5, UPB_SIZE(4, 4), 0, 0, 14, _UPB_MODE_SCALAR},
  {6, UPB_SIZE(24, 40), 5, 1, 11, _UPB_MODE_SCALAR},
};

const upb_msglayout envoy_config_core_v3_HttpProtocolOptions_msginit = {
  &envoy_config_core_v3_HttpProtocolOptions_submsgs[0],
  &envoy_config_core_v3_HttpProtocolOptions__fields[0],
  UPB_SIZE(32, 48), 6, false, 6, 255,
};

static const upb_msglayout *const envoy_config_core_v3_Http1ProtocolOptions_submsgs[2] = {
  &envoy_config_core_v3_Http1ProtocolOptions_HeaderKeyFormat_msginit,
  &google_protobuf_BoolValue_msginit,
};

static const upb_msglayout_field envoy_config_core_v3_Http1ProtocolOptions__fields[7] = {
  {1, UPB_SIZE(12, 24), 1, 1, 11, _UPB_MODE_SCALAR},
  {2, UPB_SIZE(1, 1), 0, 0, 8, _UPB_MODE_SCALAR},
  {3, UPB_SIZE(4, 8), 0, 0, 9, _UPB_MODE_SCALAR},
  {4, UPB_SIZE(16, 32), 2, 0, 11, _UPB_MODE_SCALAR},
  {5, UPB_SIZE(2, 2), 0, 0, 8, _UPB_MODE_SCALAR},
  {6, UPB_SIZE(3, 3), 0, 0, 8, _UPB_MODE_SCALAR},
  {7, UPB_SIZE(20, 40), 3, 1, 11, _UPB_MODE_SCALAR},
};

const upb_msglayout envoy_config_core_v3_Http1ProtocolOptions_msginit = {
  &envoy_config_core_v3_Http1ProtocolOptions_submsgs[0],
  &envoy_config_core_v3_Http1ProtocolOptions__fields[0],
  UPB_SIZE(24, 48), 7, false, 7, 255,
};

static const upb_msglayout *const envoy_config_core_v3_Http1ProtocolOptions_HeaderKeyFormat_submsgs[2] = {
  &envoy_config_core_v3_Http1ProtocolOptions_HeaderKeyFormat_ProperCaseWords_msginit,
  &envoy_config_core_v3_TypedExtensionConfig_msginit,
};

static const upb_msglayout_field envoy_config_core_v3_Http1ProtocolOptions_HeaderKeyFormat__fields[2] = {
  {1, UPB_SIZE(0, 0), UPB_SIZE(-5, -9), 0, 11, _UPB_MODE_SCALAR},
  {8, UPB_SIZE(0, 0), UPB_SIZE(-5, -9), 1, 11, _UPB_MODE_SCALAR},
};

const upb_msglayout envoy_config_core_v3_Http1ProtocolOptions_HeaderKeyFormat_msginit = {
  &envoy_config_core_v3_Http1ProtocolOptions_HeaderKeyFormat_submsgs[0],
  &envoy_config_core_v3_Http1ProtocolOptions_HeaderKeyFormat__fields[0],
  UPB_SIZE(8, 16), 2, false, 1, 255,
};

const upb_msglayout envoy_config_core_v3_Http1ProtocolOptions_HeaderKeyFormat_ProperCaseWords_msginit = {
  NULL,
  NULL,
  UPB_SIZE(0, 0), 0, false, 0, 255,
};

static const upb_msglayout *const envoy_config_core_v3_KeepaliveSettings_submsgs[2] = {
  &envoy_type_v3_Percent_msginit,
  &google_protobuf_Duration_msginit,
};

static const upb_msglayout_field envoy_config_core_v3_KeepaliveSettings__fields[4] = {
  {1, UPB_SIZE(4, 8), 1, 1, 11, _UPB_MODE_SCALAR},
  {2, UPB_SIZE(8, 16), 2, 1, 11, _UPB_MODE_SCALAR},
  {3, UPB_SIZE(12, 24), 3, 0, 11, _UPB_MODE_SCALAR},
  {4, UPB_SIZE(16, 32), 4, 1, 11, _UPB_MODE_SCALAR},
};

const upb_msglayout envoy_config_core_v3_KeepaliveSettings_msginit = {
  &envoy_config_core_v3_KeepaliveSettings_submsgs[0],
  &envoy_config_core_v3_KeepaliveSettings__fields[0],
  UPB_SIZE(24, 40), 4, false, 4, 255,
};

static const upb_msglayout *const envoy_config_core_v3_Http2ProtocolOptions_submsgs[4] = {
  &envoy_config_core_v3_Http2ProtocolOptions_SettingsParameter_msginit,
  &envoy_config_core_v3_KeepaliveSettings_msginit,
  &google_protobuf_BoolValue_msginit,
  &google_protobuf_UInt32Value_msginit,
};

static const upb_msglayout_field envoy_config_core_v3_Http2ProtocolOptions__fields[15] = {
  {1, UPB_SIZE(8, 8), 1, 3, 11, _UPB_MODE_SCALAR},
  {2, UPB_SIZE(12, 16), 2, 3, 11, _UPB_MODE_SCALAR},
  {3, UPB_SIZE(16, 24), 3, 3, 11, _UPB_MODE_SCALAR},
  {4, UPB_SIZE(20, 32), 4, 3, 11, _UPB_MODE_SCALAR},
  {5, UPB_SIZE(2, 2), 0, 0, 8, _UPB_MODE_SCALAR},
  {6, UPB_SIZE(3, 3), 0, 0, 8, _UPB_MODE_SCALAR},
  {7, UPB_SIZE(24, 40), 5, 3, 11, _UPB_MODE_SCALAR},
  {8, UPB_SIZE(28, 48), 6, 3, 11, _UPB_MODE_SCALAR},
  {9, UPB_SIZE(32, 56), 7, 3, 11, _UPB_MODE_SCALAR},
  {10, UPB_SIZE(36, 64), 8, 3, 11, _UPB_MODE_SCALAR},
  {11, UPB_SIZE(40, 72), 9, 3, 11, _UPB_MODE_SCALAR},
  {12, UPB_SIZE(4, 4), 0, 0, 8, _UPB_MODE_SCALAR},
  {13, UPB_SIZE(52, 96), 0, 0, 11, _UPB_MODE_ARRAY},
  {14, UPB_SIZE(44, 80), 10, 2, 11, _UPB_MODE_SCALAR},
  {15, UPB_SIZE(48, 88), 11, 1, 11, _UPB_MODE_SCALAR},
};

const upb_msglayout envoy_config_core_v3_Http2ProtocolOptions_msginit = {
  &envoy_config_core_v3_Http2ProtocolOptions_submsgs[0],
  &envoy_config_core_v3_Http2ProtocolOptions__fields[0],
  UPB_SIZE(56, 104), 15, false, 15, 255,
};

static const upb_msglayout *const envoy_config_core_v3_Http2ProtocolOptions_SettingsParameter_submsgs[1] = {
  &google_protobuf_UInt32Value_msginit,
};

static const upb_msglayout_field envoy_config_core_v3_Http2ProtocolOptions_SettingsParameter__fields[2] = {
  {1, UPB_SIZE(4, 8), 1, 0, 11, _UPB_MODE_SCALAR},
  {2, UPB_SIZE(8, 16), 2, 0, 11, _UPB_MODE_SCALAR},
};

const upb_msglayout envoy_config_core_v3_Http2ProtocolOptions_SettingsParameter_msginit = {
  &envoy_config_core_v3_Http2ProtocolOptions_SettingsParameter_submsgs[0],
  &envoy_config_core_v3_Http2ProtocolOptions_SettingsParameter__fields[0],
  UPB_SIZE(16, 24), 2, false, 2, 255,
};

static const upb_msglayout *const envoy_config_core_v3_GrpcProtocolOptions_submsgs[1] = {
  &envoy_config_core_v3_Http2ProtocolOptions_msginit,
};

static const upb_msglayout_field envoy_config_core_v3_GrpcProtocolOptions__fields[1] = {
  {1, UPB_SIZE(4, 8), 1, 0, 11, _UPB_MODE_SCALAR},
};

const upb_msglayout envoy_config_core_v3_GrpcProtocolOptions_msginit = {
  &envoy_config_core_v3_GrpcProtocolOptions_submsgs[0],
  &envoy_config_core_v3_GrpcProtocolOptions__fields[0],
  UPB_SIZE(8, 16), 1, false, 1, 255,
};

static const upb_msglayout *const envoy_config_core_v3_Http3ProtocolOptions_submsgs[2] = {
  &envoy_config_core_v3_QuicProtocolOptions_msginit,
  &google_protobuf_BoolValue_msginit,
};

static const upb_msglayout_field envoy_config_core_v3_Http3ProtocolOptions__fields[2] = {
  {1, UPB_SIZE(4, 8), 1, 0, 11, _UPB_MODE_SCALAR},
  {2, UPB_SIZE(8, 16), 2, 1, 11, _UPB_MODE_SCALAR},
};

const upb_msglayout envoy_config_core_v3_Http3ProtocolOptions_msginit = {
  &envoy_config_core_v3_Http3ProtocolOptions_submsgs[0],
  &envoy_config_core_v3_Http3ProtocolOptions__fields[0],
  UPB_SIZE(16, 24), 2, false, 2, 255,
};

static const upb_msglayout_field envoy_config_core_v3_SchemeHeaderTransformation__fields[1] = {
  {1, UPB_SIZE(0, 0), UPB_SIZE(-9, -17), 0, 9, _UPB_MODE_SCALAR},
};

const upb_msglayout envoy_config_core_v3_SchemeHeaderTransformation_msginit = {
  NULL,
  &envoy_config_core_v3_SchemeHeaderTransformation__fields[0],
  UPB_SIZE(16, 32), 1, false, 1, 255,
};

#include "upb/port_undef.inc"

