/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/config/core/v3/resolver.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include <stddef.h>
#include "upb/msg.h"
#include "envoy/config/core/v3/resolver.upb.h"
#include "envoy/config/core/v3/address.upb.h"
#include "udpa/annotations/status.upb.h"
#include "validate/validate.upb.h"

#include "upb/port_def.inc"

static const upb_msglayout_field envoy_config_core_v3_DnsResolverOptions__fields[2] = {
  {1, UPB_SIZE(0, 0), 0, 0, 8, 1},
  {2, UPB_SIZE(1, 1), 0, 0, 8, 1},
};

const upb_msglayout envoy_config_core_v3_DnsResolverOptions_msginit = {
  NULL,
  &envoy_config_core_v3_DnsResolverOptions__fields[0],
  UPB_SIZE(8, 8), 2, false, 255,
};

static const upb_msglayout *const envoy_config_core_v3_DnsResolutionConfig_submsgs[2] = {
  &envoy_config_core_v3_Address_msginit,
  &envoy_config_core_v3_DnsResolverOptions_msginit,
};

static const upb_msglayout_field envoy_config_core_v3_DnsResolutionConfig__fields[2] = {
  {1, UPB_SIZE(8, 16), 0, 0, 11, 3},
  {2, UPB_SIZE(4, 8), 1, 1, 11, 1},
};

const upb_msglayout envoy_config_core_v3_DnsResolutionConfig_msginit = {
  &envoy_config_core_v3_DnsResolutionConfig_submsgs[0],
  &envoy_config_core_v3_DnsResolutionConfig__fields[0],
  UPB_SIZE(16, 24), 2, false, 255,
};

#include "upb/port_undef.inc"

