/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/type/tracing/v2/custom_tag.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#ifndef ENVOY_TYPE_TRACING_V2_CUSTOM_TAG_PROTO_UPBDEFS_H_
#define ENVOY_TYPE_TRACING_V2_CUSTOM_TAG_PROTO_UPBDEFS_H_

#include "upb/def.h"
#include "upb/port_def.inc"
#ifdef __cplusplus
extern "C" {
#endif

#include "upb/def.h"

#include "upb/port_def.inc"

extern upb_def_init envoy_type_tracing_v2_custom_tag_proto_upbdefinit;

UPB_INLINE const upb_msgdef *envoy_type_tracing_v2_CustomTag_getmsgdef(upb_symtab *s) {
  _upb_symtab_loaddefinit(s, &envoy_type_tracing_v2_custom_tag_proto_upbdefinit);
  return upb_symtab_lookupmsg(s, "envoy.type.tracing.v2.CustomTag");
}

UPB_INLINE const upb_msgdef *envoy_type_tracing_v2_CustomTag_Literal_getmsgdef(upb_symtab *s) {
  _upb_symtab_loaddefinit(s, &envoy_type_tracing_v2_custom_tag_proto_upbdefinit);
  return upb_symtab_lookupmsg(s, "envoy.type.tracing.v2.CustomTag.Literal");
}

UPB_INLINE const upb_msgdef *envoy_type_tracing_v2_CustomTag_Environment_getmsgdef(upb_symtab *s) {
  _upb_symtab_loaddefinit(s, &envoy_type_tracing_v2_custom_tag_proto_upbdefinit);
  return upb_symtab_lookupmsg(s, "envoy.type.tracing.v2.CustomTag.Environment");
}

UPB_INLINE const upb_msgdef *envoy_type_tracing_v2_CustomTag_Header_getmsgdef(upb_symtab *s) {
  _upb_symtab_loaddefinit(s, &envoy_type_tracing_v2_custom_tag_proto_upbdefinit);
  return upb_symtab_lookupmsg(s, "envoy.type.tracing.v2.CustomTag.Header");
}

UPB_INLINE const upb_msgdef *envoy_type_tracing_v2_CustomTag_Metadata_getmsgdef(upb_symtab *s) {
  _upb_symtab_loaddefinit(s, &envoy_type_tracing_v2_custom_tag_proto_upbdefinit);
  return upb_symtab_lookupmsg(s, "envoy.type.tracing.v2.CustomTag.Metadata");
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#include "upb/port_undef.inc"

#endif  /* ENVOY_TYPE_TRACING_V2_CUSTOM_TAG_PROTO_UPBDEFS_H_ */
