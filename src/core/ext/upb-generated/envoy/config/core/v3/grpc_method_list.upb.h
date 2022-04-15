/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/config/core/v3/grpc_method_list.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#ifndef ENVOY_CONFIG_CORE_V3_GRPC_METHOD_LIST_PROTO_UPB_H_
#define ENVOY_CONFIG_CORE_V3_GRPC_METHOD_LIST_PROTO_UPB_H_

#include "upb/msg_internal.h"
#include "upb/decode.h"
#include "upb/decode_fast.h"
#include "upb/encode.h"

#include "upb/port_def.inc"

#ifdef __cplusplus
extern "C" {
#endif

struct envoy_config_core_v3_GrpcMethodList;
struct envoy_config_core_v3_GrpcMethodList_Service;
typedef struct envoy_config_core_v3_GrpcMethodList envoy_config_core_v3_GrpcMethodList;
typedef struct envoy_config_core_v3_GrpcMethodList_Service envoy_config_core_v3_GrpcMethodList_Service;
extern const upb_MiniTable envoy_config_core_v3_GrpcMethodList_msginit;
extern const upb_MiniTable envoy_config_core_v3_GrpcMethodList_Service_msginit;



/* envoy.config.core.v3.GrpcMethodList */

UPB_INLINE envoy_config_core_v3_GrpcMethodList* envoy_config_core_v3_GrpcMethodList_new(upb_Arena* arena) {
  return (envoy_config_core_v3_GrpcMethodList*)_upb_Message_New(&envoy_config_core_v3_GrpcMethodList_msginit, arena);
}
UPB_INLINE envoy_config_core_v3_GrpcMethodList* envoy_config_core_v3_GrpcMethodList_parse(const char* buf, size_t size, upb_Arena* arena) {
  envoy_config_core_v3_GrpcMethodList* ret = envoy_config_core_v3_GrpcMethodList_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &envoy_config_core_v3_GrpcMethodList_msginit, NULL, 0, arena) != kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE envoy_config_core_v3_GrpcMethodList* envoy_config_core_v3_GrpcMethodList_parse_ex(const char* buf, size_t size,
                           const upb_ExtensionRegistry* extreg,
                           int options, upb_Arena* arena) {
  envoy_config_core_v3_GrpcMethodList* ret = envoy_config_core_v3_GrpcMethodList_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &envoy_config_core_v3_GrpcMethodList_msginit, extreg, options, arena) !=
      kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE char* envoy_config_core_v3_GrpcMethodList_serialize(const envoy_config_core_v3_GrpcMethodList* msg, upb_Arena* arena, size_t* len) {
  return upb_Encode(msg, &envoy_config_core_v3_GrpcMethodList_msginit, 0, arena, len);
}
UPB_INLINE char* envoy_config_core_v3_GrpcMethodList_serialize_ex(const envoy_config_core_v3_GrpcMethodList* msg, int options,
                                 upb_Arena* arena, size_t* len) {
  return upb_Encode(msg, &envoy_config_core_v3_GrpcMethodList_msginit, options, arena, len);
}
UPB_INLINE bool envoy_config_core_v3_GrpcMethodList_has_services(const envoy_config_core_v3_GrpcMethodList* msg) {
  return _upb_has_submsg_nohasbit(msg, UPB_SIZE(0, 0));
}
UPB_INLINE void envoy_config_core_v3_GrpcMethodList_clear_services(const envoy_config_core_v3_GrpcMethodList* msg) {
  _upb_array_detach(msg, UPB_SIZE(0, 0));
}
UPB_INLINE const envoy_config_core_v3_GrpcMethodList_Service* const* envoy_config_core_v3_GrpcMethodList_services(const envoy_config_core_v3_GrpcMethodList* msg, size_t* len) {
  return (const envoy_config_core_v3_GrpcMethodList_Service* const*)_upb_array_accessor(msg, UPB_SIZE(0, 0), len);
}

UPB_INLINE envoy_config_core_v3_GrpcMethodList_Service** envoy_config_core_v3_GrpcMethodList_mutable_services(envoy_config_core_v3_GrpcMethodList* msg, size_t* len) {
  return (envoy_config_core_v3_GrpcMethodList_Service**)_upb_array_mutable_accessor(msg, UPB_SIZE(0, 0), len);
}
UPB_INLINE envoy_config_core_v3_GrpcMethodList_Service** envoy_config_core_v3_GrpcMethodList_resize_services(envoy_config_core_v3_GrpcMethodList* msg, size_t len, upb_Arena* arena) {
  return (envoy_config_core_v3_GrpcMethodList_Service**)_upb_Array_Resize_accessor2(msg, UPB_SIZE(0, 0), len, UPB_SIZE(2, 3), arena);
}
UPB_INLINE struct envoy_config_core_v3_GrpcMethodList_Service* envoy_config_core_v3_GrpcMethodList_add_services(envoy_config_core_v3_GrpcMethodList* msg, upb_Arena* arena) {
  struct envoy_config_core_v3_GrpcMethodList_Service* sub = (struct envoy_config_core_v3_GrpcMethodList_Service*)_upb_Message_New(&envoy_config_core_v3_GrpcMethodList_Service_msginit, arena);
  bool ok = _upb_Array_Append_accessor2(msg, UPB_SIZE(0, 0), UPB_SIZE(2, 3), &sub, arena);
  if (!ok) return NULL;
  return sub;
}

/* envoy.config.core.v3.GrpcMethodList.Service */

UPB_INLINE envoy_config_core_v3_GrpcMethodList_Service* envoy_config_core_v3_GrpcMethodList_Service_new(upb_Arena* arena) {
  return (envoy_config_core_v3_GrpcMethodList_Service*)_upb_Message_New(&envoy_config_core_v3_GrpcMethodList_Service_msginit, arena);
}
UPB_INLINE envoy_config_core_v3_GrpcMethodList_Service* envoy_config_core_v3_GrpcMethodList_Service_parse(const char* buf, size_t size, upb_Arena* arena) {
  envoy_config_core_v3_GrpcMethodList_Service* ret = envoy_config_core_v3_GrpcMethodList_Service_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &envoy_config_core_v3_GrpcMethodList_Service_msginit, NULL, 0, arena) != kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE envoy_config_core_v3_GrpcMethodList_Service* envoy_config_core_v3_GrpcMethodList_Service_parse_ex(const char* buf, size_t size,
                           const upb_ExtensionRegistry* extreg,
                           int options, upb_Arena* arena) {
  envoy_config_core_v3_GrpcMethodList_Service* ret = envoy_config_core_v3_GrpcMethodList_Service_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &envoy_config_core_v3_GrpcMethodList_Service_msginit, extreg, options, arena) !=
      kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE char* envoy_config_core_v3_GrpcMethodList_Service_serialize(const envoy_config_core_v3_GrpcMethodList_Service* msg, upb_Arena* arena, size_t* len) {
  return upb_Encode(msg, &envoy_config_core_v3_GrpcMethodList_Service_msginit, 0, arena, len);
}
UPB_INLINE char* envoy_config_core_v3_GrpcMethodList_Service_serialize_ex(const envoy_config_core_v3_GrpcMethodList_Service* msg, int options,
                                 upb_Arena* arena, size_t* len) {
  return upb_Encode(msg, &envoy_config_core_v3_GrpcMethodList_Service_msginit, options, arena, len);
}
UPB_INLINE void envoy_config_core_v3_GrpcMethodList_Service_clear_name(const envoy_config_core_v3_GrpcMethodList_Service* msg) {
  *UPB_PTR_AT(msg, UPB_SIZE(0, 0), upb_StringView) = upb_StringView_FromDataAndSize(NULL, 0);
}
UPB_INLINE upb_StringView envoy_config_core_v3_GrpcMethodList_Service_name(const envoy_config_core_v3_GrpcMethodList_Service* msg) {
  return *UPB_PTR_AT(msg, UPB_SIZE(0, 0), upb_StringView);
}
UPB_INLINE void envoy_config_core_v3_GrpcMethodList_Service_clear_method_names(const envoy_config_core_v3_GrpcMethodList_Service* msg) {
  _upb_array_detach(msg, UPB_SIZE(8, 16));
}
UPB_INLINE upb_StringView const* envoy_config_core_v3_GrpcMethodList_Service_method_names(const envoy_config_core_v3_GrpcMethodList_Service* msg, size_t* len) {
  return (upb_StringView const*)_upb_array_accessor(msg, UPB_SIZE(8, 16), len);
}

UPB_INLINE void envoy_config_core_v3_GrpcMethodList_Service_set_name(envoy_config_core_v3_GrpcMethodList_Service *msg, upb_StringView value) {
  *UPB_PTR_AT(msg, UPB_SIZE(0, 0), upb_StringView) = value;
}
UPB_INLINE upb_StringView* envoy_config_core_v3_GrpcMethodList_Service_mutable_method_names(envoy_config_core_v3_GrpcMethodList_Service* msg, size_t* len) {
  return (upb_StringView*)_upb_array_mutable_accessor(msg, UPB_SIZE(8, 16), len);
}
UPB_INLINE upb_StringView* envoy_config_core_v3_GrpcMethodList_Service_resize_method_names(envoy_config_core_v3_GrpcMethodList_Service* msg, size_t len, upb_Arena* arena) {
  return (upb_StringView*)_upb_Array_Resize_accessor2(msg, UPB_SIZE(8, 16), len, UPB_SIZE(3, 4), arena);
}
UPB_INLINE bool envoy_config_core_v3_GrpcMethodList_Service_add_method_names(envoy_config_core_v3_GrpcMethodList_Service* msg, upb_StringView val, upb_Arena* arena) {
  return _upb_Array_Append_accessor2(msg, UPB_SIZE(8, 16), UPB_SIZE(3, 4), &val, arena);
}

extern const upb_MiniTable_File envoy_config_core_v3_grpc_method_list_proto_upb_file_layout;

#ifdef __cplusplus
}  /* extern "C" */
#endif

#include "upb/port_undef.inc"

#endif  /* ENVOY_CONFIG_CORE_V3_GRPC_METHOD_LIST_PROTO_UPB_H_ */
