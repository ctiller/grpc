/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     udpa/type/v1/typed_struct.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#ifndef UDPA_TYPE_V1_TYPED_STRUCT_PROTO_UPB_H_
#define UDPA_TYPE_V1_TYPED_STRUCT_PROTO_UPB_H_

#include "upb/msg_internal.h"
#include "upb/decode.h"
#include "upb/decode_fast.h"
#include "upb/encode.h"

#include "upb/port_def.inc"

#ifdef __cplusplus
extern "C" {
#endif

struct udpa_type_v1_TypedStruct;
typedef struct udpa_type_v1_TypedStruct udpa_type_v1_TypedStruct;
extern const upb_msglayout udpa_type_v1_TypedStruct_msginit;
struct google_protobuf_Struct;
extern const upb_msglayout google_protobuf_Struct_msginit;


/* udpa.type.v1.TypedStruct */

UPB_INLINE udpa_type_v1_TypedStruct *udpa_type_v1_TypedStruct_new(upb_arena *arena) {
  return (udpa_type_v1_TypedStruct *)_upb_msg_new(&udpa_type_v1_TypedStruct_msginit, arena);
}
UPB_INLINE udpa_type_v1_TypedStruct *udpa_type_v1_TypedStruct_parse(const char *buf, size_t size,
                        upb_arena *arena) {
  udpa_type_v1_TypedStruct *ret = udpa_type_v1_TypedStruct_new(arena);
  if (!ret) return NULL;
  if (!upb_decode(buf, size, ret, &udpa_type_v1_TypedStruct_msginit, arena)) return NULL;
  return ret;
}
UPB_INLINE udpa_type_v1_TypedStruct *udpa_type_v1_TypedStruct_parse_ex(const char *buf, size_t size,
                           const upb_extreg *extreg, int options,
                           upb_arena *arena) {
  udpa_type_v1_TypedStruct *ret = udpa_type_v1_TypedStruct_new(arena);
  if (!ret) return NULL;
  if (!_upb_decode(buf, size, ret, &udpa_type_v1_TypedStruct_msginit, extreg, options, arena)) {
    return NULL;
  }
  return ret;
}
UPB_INLINE char *udpa_type_v1_TypedStruct_serialize(const udpa_type_v1_TypedStruct *msg, upb_arena *arena, size_t *len) {
  return upb_encode(msg, &udpa_type_v1_TypedStruct_msginit, arena, len);
}

UPB_INLINE upb_strview udpa_type_v1_TypedStruct_type_url(const udpa_type_v1_TypedStruct *msg) { return *UPB_PTR_AT(msg, UPB_SIZE(4, 8), upb_strview); }
UPB_INLINE bool udpa_type_v1_TypedStruct_has_value(const udpa_type_v1_TypedStruct *msg) { return _upb_hasbit(msg, 1); }
UPB_INLINE const struct google_protobuf_Struct* udpa_type_v1_TypedStruct_value(const udpa_type_v1_TypedStruct *msg) { return *UPB_PTR_AT(msg, UPB_SIZE(12, 24), const struct google_protobuf_Struct*); }

UPB_INLINE void udpa_type_v1_TypedStruct_set_type_url(udpa_type_v1_TypedStruct *msg, upb_strview value) {
  *UPB_PTR_AT(msg, UPB_SIZE(4, 8), upb_strview) = value;
}
UPB_INLINE void udpa_type_v1_TypedStruct_set_value(udpa_type_v1_TypedStruct *msg, struct google_protobuf_Struct* value) {
  _upb_sethas(msg, 1);
  *UPB_PTR_AT(msg, UPB_SIZE(12, 24), struct google_protobuf_Struct*) = value;
}
UPB_INLINE struct google_protobuf_Struct* udpa_type_v1_TypedStruct_mutable_value(udpa_type_v1_TypedStruct *msg, upb_arena *arena) {
  struct google_protobuf_Struct* sub = (struct google_protobuf_Struct*)udpa_type_v1_TypedStruct_value(msg);
  if (sub == NULL) {
    sub = (struct google_protobuf_Struct*)_upb_msg_new(&google_protobuf_Struct_msginit, arena);
    if (!sub) return NULL;
    udpa_type_v1_TypedStruct_set_value(msg, sub);
  }
  return sub;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#include "upb/port_undef.inc"

#endif  /* UDPA_TYPE_V1_TYPED_STRUCT_PROTO_UPB_H_ */
