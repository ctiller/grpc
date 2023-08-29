/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/config/cluster/v3/circuit_breaker.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#ifndef ENVOY_CONFIG_CLUSTER_V3_CIRCUIT_BREAKER_PROTO_UPB_H_
#define ENVOY_CONFIG_CLUSTER_V3_CIRCUIT_BREAKER_PROTO_UPB_H_

#include "upb/generated_code_support.h"
// Must be last. 
#include "upb/port/def.inc"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct envoy_config_cluster_v3_CircuitBreakers envoy_config_cluster_v3_CircuitBreakers;
typedef struct envoy_config_cluster_v3_CircuitBreakers_Thresholds envoy_config_cluster_v3_CircuitBreakers_Thresholds;
typedef struct envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget;
extern const upb_MiniTable envoy_config_cluster_v3_CircuitBreakers_msg_init;
extern const upb_MiniTable envoy_config_cluster_v3_CircuitBreakers_Thresholds_msg_init;
extern const upb_MiniTable envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_msg_init;
struct envoy_type_v3_Percent;
struct google_protobuf_UInt32Value;
extern const upb_MiniTable envoy_type_v3_Percent_msg_init;
extern const upb_MiniTable google_protobuf_UInt32Value_msg_init;



/* envoy.config.cluster.v3.CircuitBreakers */

UPB_INLINE envoy_config_cluster_v3_CircuitBreakers* envoy_config_cluster_v3_CircuitBreakers_new(upb_Arena* arena) {
  return (envoy_config_cluster_v3_CircuitBreakers*)_upb_Message_New(&envoy_config_cluster_v3_CircuitBreakers_msg_init, arena);
}
UPB_INLINE envoy_config_cluster_v3_CircuitBreakers* envoy_config_cluster_v3_CircuitBreakers_parse(const char* buf, size_t size, upb_Arena* arena) {
  envoy_config_cluster_v3_CircuitBreakers* ret = envoy_config_cluster_v3_CircuitBreakers_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &envoy_config_cluster_v3_CircuitBreakers_msg_init, NULL, 0, arena) != kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE envoy_config_cluster_v3_CircuitBreakers* envoy_config_cluster_v3_CircuitBreakers_parse_ex(const char* buf, size_t size,
                           const upb_ExtensionRegistry* extreg,
                           int options, upb_Arena* arena) {
  envoy_config_cluster_v3_CircuitBreakers* ret = envoy_config_cluster_v3_CircuitBreakers_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &envoy_config_cluster_v3_CircuitBreakers_msg_init, extreg, options, arena) !=
      kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE char* envoy_config_cluster_v3_CircuitBreakers_serialize(const envoy_config_cluster_v3_CircuitBreakers* msg, upb_Arena* arena, size_t* len) {
  char* ptr;
  (void)upb_Encode(msg, &envoy_config_cluster_v3_CircuitBreakers_msg_init, 0, arena, &ptr, len);
  return ptr;
}
UPB_INLINE char* envoy_config_cluster_v3_CircuitBreakers_serialize_ex(const envoy_config_cluster_v3_CircuitBreakers* msg, int options,
                                 upb_Arena* arena, size_t* len) {
  char* ptr;
  (void)upb_Encode(msg, &envoy_config_cluster_v3_CircuitBreakers_msg_init, options, arena, &ptr, len);
  return ptr;
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_clear_thresholds(envoy_config_cluster_v3_CircuitBreakers* msg) {
  const upb_MiniTableField field = {1, 0, 0, 0, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_ClearNonExtensionField(msg, &field);
}
UPB_INLINE const envoy_config_cluster_v3_CircuitBreakers_Thresholds* const* envoy_config_cluster_v3_CircuitBreakers_thresholds(const envoy_config_cluster_v3_CircuitBreakers* msg, size_t* size) {
  const upb_MiniTableField field = {1, 0, 0, 0, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  const upb_Array* arr = upb_Message_GetArray(msg, &field);
  if (arr) {
    if (size) *size = arr->size;
    return (const envoy_config_cluster_v3_CircuitBreakers_Thresholds* const*)_upb_array_constptr(arr);
  } else {
    if (size) *size = 0;
    return NULL;
  }
}
UPB_INLINE const upb_Array* _envoy_config_cluster_v3_CircuitBreakers_thresholds_upb_array(const envoy_config_cluster_v3_CircuitBreakers* msg, size_t* size) {
  const upb_MiniTableField field = {1, 0, 0, 0, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  const upb_Array* arr = upb_Message_GetArray(msg, &field);
  if (size) {
    *size = arr ? arr->size : 0;
  }
  return arr;
}
UPB_INLINE upb_Array* _envoy_config_cluster_v3_CircuitBreakers_thresholds_mutable_upb_array(const envoy_config_cluster_v3_CircuitBreakers* msg, size_t* size, upb_Arena* arena) {
  const upb_MiniTableField field = {1, 0, 0, 0, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  upb_Array* arr = upb_Message_GetOrCreateMutableArray(
      (upb_Message*)msg, &field, arena);
  if (size) {
    *size = arr ? arr->size : 0;
  }
  return arr;
}
UPB_INLINE bool envoy_config_cluster_v3_CircuitBreakers_has_thresholds(const envoy_config_cluster_v3_CircuitBreakers* msg) {
  size_t size;
  envoy_config_cluster_v3_CircuitBreakers_thresholds(msg, &size);
  return size != 0;
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_clear_per_host_thresholds(envoy_config_cluster_v3_CircuitBreakers* msg) {
  const upb_MiniTableField field = {2, UPB_SIZE(4, 8), 0, 1, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_ClearNonExtensionField(msg, &field);
}
UPB_INLINE const envoy_config_cluster_v3_CircuitBreakers_Thresholds* const* envoy_config_cluster_v3_CircuitBreakers_per_host_thresholds(const envoy_config_cluster_v3_CircuitBreakers* msg, size_t* size) {
  const upb_MiniTableField field = {2, UPB_SIZE(4, 8), 0, 1, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  const upb_Array* arr = upb_Message_GetArray(msg, &field);
  if (arr) {
    if (size) *size = arr->size;
    return (const envoy_config_cluster_v3_CircuitBreakers_Thresholds* const*)_upb_array_constptr(arr);
  } else {
    if (size) *size = 0;
    return NULL;
  }
}
UPB_INLINE const upb_Array* _envoy_config_cluster_v3_CircuitBreakers_per_host_thresholds_upb_array(const envoy_config_cluster_v3_CircuitBreakers* msg, size_t* size) {
  const upb_MiniTableField field = {2, UPB_SIZE(4, 8), 0, 1, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  const upb_Array* arr = upb_Message_GetArray(msg, &field);
  if (size) {
    *size = arr ? arr->size : 0;
  }
  return arr;
}
UPB_INLINE upb_Array* _envoy_config_cluster_v3_CircuitBreakers_per_host_thresholds_mutable_upb_array(const envoy_config_cluster_v3_CircuitBreakers* msg, size_t* size, upb_Arena* arena) {
  const upb_MiniTableField field = {2, UPB_SIZE(4, 8), 0, 1, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  upb_Array* arr = upb_Message_GetOrCreateMutableArray(
      (upb_Message*)msg, &field, arena);
  if (size) {
    *size = arr ? arr->size : 0;
  }
  return arr;
}
UPB_INLINE bool envoy_config_cluster_v3_CircuitBreakers_has_per_host_thresholds(const envoy_config_cluster_v3_CircuitBreakers* msg) {
  size_t size;
  envoy_config_cluster_v3_CircuitBreakers_per_host_thresholds(msg, &size);
  return size != 0;
}

UPB_INLINE envoy_config_cluster_v3_CircuitBreakers_Thresholds** envoy_config_cluster_v3_CircuitBreakers_mutable_thresholds(envoy_config_cluster_v3_CircuitBreakers* msg, size_t* size) {
  upb_MiniTableField field = {1, 0, 0, 0, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  upb_Array* arr = upb_Message_GetMutableArray(msg, &field);
  if (arr) {
    if (size) *size = arr->size;
    return (envoy_config_cluster_v3_CircuitBreakers_Thresholds**)_upb_array_ptr(arr);
  } else {
    if (size) *size = 0;
    return NULL;
  }
}
UPB_INLINE envoy_config_cluster_v3_CircuitBreakers_Thresholds** envoy_config_cluster_v3_CircuitBreakers_resize_thresholds(envoy_config_cluster_v3_CircuitBreakers* msg, size_t size, upb_Arena* arena) {
  upb_MiniTableField field = {1, 0, 0, 0, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  return (envoy_config_cluster_v3_CircuitBreakers_Thresholds**)upb_Message_ResizeArrayUninitialized(msg, &field, size, arena);
}
UPB_INLINE struct envoy_config_cluster_v3_CircuitBreakers_Thresholds* envoy_config_cluster_v3_CircuitBreakers_add_thresholds(envoy_config_cluster_v3_CircuitBreakers* msg, upb_Arena* arena) {
  upb_MiniTableField field = {1, 0, 0, 0, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  upb_Array* arr = upb_Message_GetOrCreateMutableArray(msg, &field, arena);
  if (!arr || !_upb_Array_ResizeUninitialized(arr, arr->size + 1, arena)) {
    return NULL;
  }
  struct envoy_config_cluster_v3_CircuitBreakers_Thresholds* sub = (struct envoy_config_cluster_v3_CircuitBreakers_Thresholds*)_upb_Message_New(&envoy_config_cluster_v3_CircuitBreakers_Thresholds_msg_init, arena);
  if (!arr || !sub) return NULL;
  _upb_Array_Set(arr, arr->size - 1, &sub, sizeof(sub));
  return sub;
}
UPB_INLINE envoy_config_cluster_v3_CircuitBreakers_Thresholds** envoy_config_cluster_v3_CircuitBreakers_mutable_per_host_thresholds(envoy_config_cluster_v3_CircuitBreakers* msg, size_t* size) {
  upb_MiniTableField field = {2, UPB_SIZE(4, 8), 0, 1, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  upb_Array* arr = upb_Message_GetMutableArray(msg, &field);
  if (arr) {
    if (size) *size = arr->size;
    return (envoy_config_cluster_v3_CircuitBreakers_Thresholds**)_upb_array_ptr(arr);
  } else {
    if (size) *size = 0;
    return NULL;
  }
}
UPB_INLINE envoy_config_cluster_v3_CircuitBreakers_Thresholds** envoy_config_cluster_v3_CircuitBreakers_resize_per_host_thresholds(envoy_config_cluster_v3_CircuitBreakers* msg, size_t size, upb_Arena* arena) {
  upb_MiniTableField field = {2, UPB_SIZE(4, 8), 0, 1, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  return (envoy_config_cluster_v3_CircuitBreakers_Thresholds**)upb_Message_ResizeArrayUninitialized(msg, &field, size, arena);
}
UPB_INLINE struct envoy_config_cluster_v3_CircuitBreakers_Thresholds* envoy_config_cluster_v3_CircuitBreakers_add_per_host_thresholds(envoy_config_cluster_v3_CircuitBreakers* msg, upb_Arena* arena) {
  upb_MiniTableField field = {2, UPB_SIZE(4, 8), 0, 1, 11, (int)kUpb_FieldMode_Array | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  upb_Array* arr = upb_Message_GetOrCreateMutableArray(msg, &field, arena);
  if (!arr || !_upb_Array_ResizeUninitialized(arr, arr->size + 1, arena)) {
    return NULL;
  }
  struct envoy_config_cluster_v3_CircuitBreakers_Thresholds* sub = (struct envoy_config_cluster_v3_CircuitBreakers_Thresholds*)_upb_Message_New(&envoy_config_cluster_v3_CircuitBreakers_Thresholds_msg_init, arena);
  if (!arr || !sub) return NULL;
  _upb_Array_Set(arr, arr->size - 1, &sub, sizeof(sub));
  return sub;
}

/* envoy.config.cluster.v3.CircuitBreakers.Thresholds */

UPB_INLINE envoy_config_cluster_v3_CircuitBreakers_Thresholds* envoy_config_cluster_v3_CircuitBreakers_Thresholds_new(upb_Arena* arena) {
  return (envoy_config_cluster_v3_CircuitBreakers_Thresholds*)_upb_Message_New(&envoy_config_cluster_v3_CircuitBreakers_Thresholds_msg_init, arena);
}
UPB_INLINE envoy_config_cluster_v3_CircuitBreakers_Thresholds* envoy_config_cluster_v3_CircuitBreakers_Thresholds_parse(const char* buf, size_t size, upb_Arena* arena) {
  envoy_config_cluster_v3_CircuitBreakers_Thresholds* ret = envoy_config_cluster_v3_CircuitBreakers_Thresholds_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &envoy_config_cluster_v3_CircuitBreakers_Thresholds_msg_init, NULL, 0, arena) != kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE envoy_config_cluster_v3_CircuitBreakers_Thresholds* envoy_config_cluster_v3_CircuitBreakers_Thresholds_parse_ex(const char* buf, size_t size,
                           const upb_ExtensionRegistry* extreg,
                           int options, upb_Arena* arena) {
  envoy_config_cluster_v3_CircuitBreakers_Thresholds* ret = envoy_config_cluster_v3_CircuitBreakers_Thresholds_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &envoy_config_cluster_v3_CircuitBreakers_Thresholds_msg_init, extreg, options, arena) !=
      kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE char* envoy_config_cluster_v3_CircuitBreakers_Thresholds_serialize(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg, upb_Arena* arena, size_t* len) {
  char* ptr;
  (void)upb_Encode(msg, &envoy_config_cluster_v3_CircuitBreakers_Thresholds_msg_init, 0, arena, &ptr, len);
  return ptr;
}
UPB_INLINE char* envoy_config_cluster_v3_CircuitBreakers_Thresholds_serialize_ex(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg, int options,
                                 upb_Arena* arena, size_t* len) {
  char* ptr;
  (void)upb_Encode(msg, &envoy_config_cluster_v3_CircuitBreakers_Thresholds_msg_init, options, arena, &ptr, len);
  return ptr;
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_clear_priority(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {1, 4, 0, kUpb_NoSub, 5, (int)kUpb_FieldMode_Scalar | (int)kUpb_LabelFlags_IsAlternate | ((int)kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)};
  _upb_Message_ClearNonExtensionField(msg, &field);
}
UPB_INLINE int32_t envoy_config_cluster_v3_CircuitBreakers_Thresholds_priority(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  int32_t default_val = 0;
  int32_t ret;
  const upb_MiniTableField field = {1, 4, 0, kUpb_NoSub, 5, (int)kUpb_FieldMode_Scalar | (int)kUpb_LabelFlags_IsAlternate | ((int)kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)};
  _upb_Message_GetNonExtensionField(msg, &field, &default_val, &ret);
  return ret;
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_clear_max_connections(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {2, UPB_SIZE(8, 16), 1, 0, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_ClearNonExtensionField(msg, &field);
}
UPB_INLINE const struct google_protobuf_UInt32Value* envoy_config_cluster_v3_CircuitBreakers_Thresholds_max_connections(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const struct google_protobuf_UInt32Value* default_val = NULL;
  const struct google_protobuf_UInt32Value* ret;
  const upb_MiniTableField field = {2, UPB_SIZE(8, 16), 1, 0, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_GetNonExtensionField(msg, &field, &default_val, &ret);
  return ret;
}
UPB_INLINE bool envoy_config_cluster_v3_CircuitBreakers_Thresholds_has_max_connections(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {2, UPB_SIZE(8, 16), 1, 0, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  return _upb_Message_HasNonExtensionField(msg, &field);
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_clear_max_pending_requests(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {3, UPB_SIZE(12, 24), 2, 1, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_ClearNonExtensionField(msg, &field);
}
UPB_INLINE const struct google_protobuf_UInt32Value* envoy_config_cluster_v3_CircuitBreakers_Thresholds_max_pending_requests(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const struct google_protobuf_UInt32Value* default_val = NULL;
  const struct google_protobuf_UInt32Value* ret;
  const upb_MiniTableField field = {3, UPB_SIZE(12, 24), 2, 1, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_GetNonExtensionField(msg, &field, &default_val, &ret);
  return ret;
}
UPB_INLINE bool envoy_config_cluster_v3_CircuitBreakers_Thresholds_has_max_pending_requests(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {3, UPB_SIZE(12, 24), 2, 1, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  return _upb_Message_HasNonExtensionField(msg, &field);
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_clear_max_requests(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {4, UPB_SIZE(16, 32), 3, 2, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_ClearNonExtensionField(msg, &field);
}
UPB_INLINE const struct google_protobuf_UInt32Value* envoy_config_cluster_v3_CircuitBreakers_Thresholds_max_requests(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const struct google_protobuf_UInt32Value* default_val = NULL;
  const struct google_protobuf_UInt32Value* ret;
  const upb_MiniTableField field = {4, UPB_SIZE(16, 32), 3, 2, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_GetNonExtensionField(msg, &field, &default_val, &ret);
  return ret;
}
UPB_INLINE bool envoy_config_cluster_v3_CircuitBreakers_Thresholds_has_max_requests(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {4, UPB_SIZE(16, 32), 3, 2, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  return _upb_Message_HasNonExtensionField(msg, &field);
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_clear_max_retries(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {5, UPB_SIZE(20, 40), 4, 3, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_ClearNonExtensionField(msg, &field);
}
UPB_INLINE const struct google_protobuf_UInt32Value* envoy_config_cluster_v3_CircuitBreakers_Thresholds_max_retries(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const struct google_protobuf_UInt32Value* default_val = NULL;
  const struct google_protobuf_UInt32Value* ret;
  const upb_MiniTableField field = {5, UPB_SIZE(20, 40), 4, 3, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_GetNonExtensionField(msg, &field, &default_val, &ret);
  return ret;
}
UPB_INLINE bool envoy_config_cluster_v3_CircuitBreakers_Thresholds_has_max_retries(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {5, UPB_SIZE(20, 40), 4, 3, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  return _upb_Message_HasNonExtensionField(msg, &field);
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_clear_track_remaining(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {6, UPB_SIZE(24, 8), 0, kUpb_NoSub, 8, (int)kUpb_FieldMode_Scalar | ((int)kUpb_FieldRep_1Byte << kUpb_FieldRep_Shift)};
  _upb_Message_ClearNonExtensionField(msg, &field);
}
UPB_INLINE bool envoy_config_cluster_v3_CircuitBreakers_Thresholds_track_remaining(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  bool default_val = false;
  bool ret;
  const upb_MiniTableField field = {6, UPB_SIZE(24, 8), 0, kUpb_NoSub, 8, (int)kUpb_FieldMode_Scalar | ((int)kUpb_FieldRep_1Byte << kUpb_FieldRep_Shift)};
  _upb_Message_GetNonExtensionField(msg, &field, &default_val, &ret);
  return ret;
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_clear_max_connection_pools(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {7, UPB_SIZE(28, 48), 5, 4, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_ClearNonExtensionField(msg, &field);
}
UPB_INLINE const struct google_protobuf_UInt32Value* envoy_config_cluster_v3_CircuitBreakers_Thresholds_max_connection_pools(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const struct google_protobuf_UInt32Value* default_val = NULL;
  const struct google_protobuf_UInt32Value* ret;
  const upb_MiniTableField field = {7, UPB_SIZE(28, 48), 5, 4, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_GetNonExtensionField(msg, &field, &default_val, &ret);
  return ret;
}
UPB_INLINE bool envoy_config_cluster_v3_CircuitBreakers_Thresholds_has_max_connection_pools(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {7, UPB_SIZE(28, 48), 5, 4, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  return _upb_Message_HasNonExtensionField(msg, &field);
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_clear_retry_budget(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {8, UPB_SIZE(32, 56), 6, 5, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_ClearNonExtensionField(msg, &field);
}
UPB_INLINE const envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* envoy_config_cluster_v3_CircuitBreakers_Thresholds_retry_budget(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* default_val = NULL;
  const envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* ret;
  const upb_MiniTableField field = {8, UPB_SIZE(32, 56), 6, 5, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_GetNonExtensionField(msg, &field, &default_val, &ret);
  return ret;
}
UPB_INLINE bool envoy_config_cluster_v3_CircuitBreakers_Thresholds_has_retry_budget(const envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg) {
  const upb_MiniTableField field = {8, UPB_SIZE(32, 56), 6, 5, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  return _upb_Message_HasNonExtensionField(msg, &field);
}

UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_priority(envoy_config_cluster_v3_CircuitBreakers_Thresholds *msg, int32_t value) {
  const upb_MiniTableField field = {1, 4, 0, kUpb_NoSub, 5, (int)kUpb_FieldMode_Scalar | (int)kUpb_LabelFlags_IsAlternate | ((int)kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)};
  _upb_Message_SetNonExtensionField(msg, &field, &value);
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_max_connections(envoy_config_cluster_v3_CircuitBreakers_Thresholds *msg, struct google_protobuf_UInt32Value* value) {
  const upb_MiniTableField field = {2, UPB_SIZE(8, 16), 1, 0, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_SetNonExtensionField(msg, &field, &value);
}
UPB_INLINE struct google_protobuf_UInt32Value* envoy_config_cluster_v3_CircuitBreakers_Thresholds_mutable_max_connections(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg, upb_Arena* arena) {
  struct google_protobuf_UInt32Value* sub = (struct google_protobuf_UInt32Value*)envoy_config_cluster_v3_CircuitBreakers_Thresholds_max_connections(msg);
  if (sub == NULL) {
    sub = (struct google_protobuf_UInt32Value*)_upb_Message_New(&google_protobuf_UInt32Value_msg_init, arena);
    if (sub) envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_max_connections(msg, sub);
  }
  return sub;
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_max_pending_requests(envoy_config_cluster_v3_CircuitBreakers_Thresholds *msg, struct google_protobuf_UInt32Value* value) {
  const upb_MiniTableField field = {3, UPB_SIZE(12, 24), 2, 1, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_SetNonExtensionField(msg, &field, &value);
}
UPB_INLINE struct google_protobuf_UInt32Value* envoy_config_cluster_v3_CircuitBreakers_Thresholds_mutable_max_pending_requests(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg, upb_Arena* arena) {
  struct google_protobuf_UInt32Value* sub = (struct google_protobuf_UInt32Value*)envoy_config_cluster_v3_CircuitBreakers_Thresholds_max_pending_requests(msg);
  if (sub == NULL) {
    sub = (struct google_protobuf_UInt32Value*)_upb_Message_New(&google_protobuf_UInt32Value_msg_init, arena);
    if (sub) envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_max_pending_requests(msg, sub);
  }
  return sub;
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_max_requests(envoy_config_cluster_v3_CircuitBreakers_Thresholds *msg, struct google_protobuf_UInt32Value* value) {
  const upb_MiniTableField field = {4, UPB_SIZE(16, 32), 3, 2, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_SetNonExtensionField(msg, &field, &value);
}
UPB_INLINE struct google_protobuf_UInt32Value* envoy_config_cluster_v3_CircuitBreakers_Thresholds_mutable_max_requests(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg, upb_Arena* arena) {
  struct google_protobuf_UInt32Value* sub = (struct google_protobuf_UInt32Value*)envoy_config_cluster_v3_CircuitBreakers_Thresholds_max_requests(msg);
  if (sub == NULL) {
    sub = (struct google_protobuf_UInt32Value*)_upb_Message_New(&google_protobuf_UInt32Value_msg_init, arena);
    if (sub) envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_max_requests(msg, sub);
  }
  return sub;
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_max_retries(envoy_config_cluster_v3_CircuitBreakers_Thresholds *msg, struct google_protobuf_UInt32Value* value) {
  const upb_MiniTableField field = {5, UPB_SIZE(20, 40), 4, 3, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_SetNonExtensionField(msg, &field, &value);
}
UPB_INLINE struct google_protobuf_UInt32Value* envoy_config_cluster_v3_CircuitBreakers_Thresholds_mutable_max_retries(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg, upb_Arena* arena) {
  struct google_protobuf_UInt32Value* sub = (struct google_protobuf_UInt32Value*)envoy_config_cluster_v3_CircuitBreakers_Thresholds_max_retries(msg);
  if (sub == NULL) {
    sub = (struct google_protobuf_UInt32Value*)_upb_Message_New(&google_protobuf_UInt32Value_msg_init, arena);
    if (sub) envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_max_retries(msg, sub);
  }
  return sub;
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_track_remaining(envoy_config_cluster_v3_CircuitBreakers_Thresholds *msg, bool value) {
  const upb_MiniTableField field = {6, UPB_SIZE(24, 8), 0, kUpb_NoSub, 8, (int)kUpb_FieldMode_Scalar | ((int)kUpb_FieldRep_1Byte << kUpb_FieldRep_Shift)};
  _upb_Message_SetNonExtensionField(msg, &field, &value);
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_max_connection_pools(envoy_config_cluster_v3_CircuitBreakers_Thresholds *msg, struct google_protobuf_UInt32Value* value) {
  const upb_MiniTableField field = {7, UPB_SIZE(28, 48), 5, 4, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_SetNonExtensionField(msg, &field, &value);
}
UPB_INLINE struct google_protobuf_UInt32Value* envoy_config_cluster_v3_CircuitBreakers_Thresholds_mutable_max_connection_pools(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg, upb_Arena* arena) {
  struct google_protobuf_UInt32Value* sub = (struct google_protobuf_UInt32Value*)envoy_config_cluster_v3_CircuitBreakers_Thresholds_max_connection_pools(msg);
  if (sub == NULL) {
    sub = (struct google_protobuf_UInt32Value*)_upb_Message_New(&google_protobuf_UInt32Value_msg_init, arena);
    if (sub) envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_max_connection_pools(msg, sub);
  }
  return sub;
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_retry_budget(envoy_config_cluster_v3_CircuitBreakers_Thresholds *msg, envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* value) {
  const upb_MiniTableField field = {8, UPB_SIZE(32, 56), 6, 5, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_SetNonExtensionField(msg, &field, &value);
}
UPB_INLINE struct envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* envoy_config_cluster_v3_CircuitBreakers_Thresholds_mutable_retry_budget(envoy_config_cluster_v3_CircuitBreakers_Thresholds* msg, upb_Arena* arena) {
  struct envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* sub = (struct envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget*)envoy_config_cluster_v3_CircuitBreakers_Thresholds_retry_budget(msg);
  if (sub == NULL) {
    sub = (struct envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget*)_upb_Message_New(&envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_msg_init, arena);
    if (sub) envoy_config_cluster_v3_CircuitBreakers_Thresholds_set_retry_budget(msg, sub);
  }
  return sub;
}

/* envoy.config.cluster.v3.CircuitBreakers.Thresholds.RetryBudget */

UPB_INLINE envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_new(upb_Arena* arena) {
  return (envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget*)_upb_Message_New(&envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_msg_init, arena);
}
UPB_INLINE envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_parse(const char* buf, size_t size, upb_Arena* arena) {
  envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* ret = envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_msg_init, NULL, 0, arena) != kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_parse_ex(const char* buf, size_t size,
                           const upb_ExtensionRegistry* extreg,
                           int options, upb_Arena* arena) {
  envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* ret = envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_msg_init, extreg, options, arena) !=
      kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE char* envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_serialize(const envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* msg, upb_Arena* arena, size_t* len) {
  char* ptr;
  (void)upb_Encode(msg, &envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_msg_init, 0, arena, &ptr, len);
  return ptr;
}
UPB_INLINE char* envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_serialize_ex(const envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* msg, int options,
                                 upb_Arena* arena, size_t* len) {
  char* ptr;
  (void)upb_Encode(msg, &envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_msg_init, options, arena, &ptr, len);
  return ptr;
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_clear_budget_percent(envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* msg) {
  const upb_MiniTableField field = {1, UPB_SIZE(4, 8), 1, 0, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_ClearNonExtensionField(msg, &field);
}
UPB_INLINE const struct envoy_type_v3_Percent* envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_budget_percent(const envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* msg) {
  const struct envoy_type_v3_Percent* default_val = NULL;
  const struct envoy_type_v3_Percent* ret;
  const upb_MiniTableField field = {1, UPB_SIZE(4, 8), 1, 0, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_GetNonExtensionField(msg, &field, &default_val, &ret);
  return ret;
}
UPB_INLINE bool envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_has_budget_percent(const envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* msg) {
  const upb_MiniTableField field = {1, UPB_SIZE(4, 8), 1, 0, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  return _upb_Message_HasNonExtensionField(msg, &field);
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_clear_min_retry_concurrency(envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* msg) {
  const upb_MiniTableField field = {2, UPB_SIZE(8, 16), 2, 1, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_ClearNonExtensionField(msg, &field);
}
UPB_INLINE const struct google_protobuf_UInt32Value* envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_min_retry_concurrency(const envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* msg) {
  const struct google_protobuf_UInt32Value* default_val = NULL;
  const struct google_protobuf_UInt32Value* ret;
  const upb_MiniTableField field = {2, UPB_SIZE(8, 16), 2, 1, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_GetNonExtensionField(msg, &field, &default_val, &ret);
  return ret;
}
UPB_INLINE bool envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_has_min_retry_concurrency(const envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* msg) {
  const upb_MiniTableField field = {2, UPB_SIZE(8, 16), 2, 1, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  return _upb_Message_HasNonExtensionField(msg, &field);
}

UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_set_budget_percent(envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget *msg, struct envoy_type_v3_Percent* value) {
  const upb_MiniTableField field = {1, UPB_SIZE(4, 8), 1, 0, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_SetNonExtensionField(msg, &field, &value);
}
UPB_INLINE struct envoy_type_v3_Percent* envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_mutable_budget_percent(envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* msg, upb_Arena* arena) {
  struct envoy_type_v3_Percent* sub = (struct envoy_type_v3_Percent*)envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_budget_percent(msg);
  if (sub == NULL) {
    sub = (struct envoy_type_v3_Percent*)_upb_Message_New(&envoy_type_v3_Percent_msg_init, arena);
    if (sub) envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_set_budget_percent(msg, sub);
  }
  return sub;
}
UPB_INLINE void envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_set_min_retry_concurrency(envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget *msg, struct google_protobuf_UInt32Value* value) {
  const upb_MiniTableField field = {2, UPB_SIZE(8, 16), 2, 1, 11, (int)kUpb_FieldMode_Scalar | ((int)UPB_SIZE(kUpb_FieldRep_4Byte, kUpb_FieldRep_8Byte) << kUpb_FieldRep_Shift)};
  _upb_Message_SetNonExtensionField(msg, &field, &value);
}
UPB_INLINE struct google_protobuf_UInt32Value* envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_mutable_min_retry_concurrency(envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget* msg, upb_Arena* arena) {
  struct google_protobuf_UInt32Value* sub = (struct google_protobuf_UInt32Value*)envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_min_retry_concurrency(msg);
  if (sub == NULL) {
    sub = (struct google_protobuf_UInt32Value*)_upb_Message_New(&google_protobuf_UInt32Value_msg_init, arena);
    if (sub) envoy_config_cluster_v3_CircuitBreakers_Thresholds_RetryBudget_set_min_retry_concurrency(msg, sub);
  }
  return sub;
}

extern const upb_MiniTableFile envoy_config_cluster_v3_circuit_breaker_proto_upb_file_layout;

#ifdef __cplusplus
}  /* extern "C" */
#endif

#include "upb/port/undef.inc"

#endif  /* ENVOY_CONFIG_CLUSTER_V3_CIRCUIT_BREAKER_PROTO_UPB_H_ */
