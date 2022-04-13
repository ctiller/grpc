/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     xds/annotations/v3/migrate.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#ifndef XDS_ANNOTATIONS_V3_MIGRATE_PROTO_UPB_H_
#define XDS_ANNOTATIONS_V3_MIGRATE_PROTO_UPB_H_

#include "upb/msg_internal.h"
#include "upb/decode.h"
#include "upb/decode_fast.h"
#include "upb/encode.h"

#include "upb/port_def.inc"

#ifdef __cplusplus
extern "C" {
#endif

struct xds_annotations_v3_MigrateAnnotation;
struct xds_annotations_v3_FieldMigrateAnnotation;
struct xds_annotations_v3_FileMigrateAnnotation;
typedef struct xds_annotations_v3_MigrateAnnotation xds_annotations_v3_MigrateAnnotation;
typedef struct xds_annotations_v3_FieldMigrateAnnotation xds_annotations_v3_FieldMigrateAnnotation;
typedef struct xds_annotations_v3_FileMigrateAnnotation xds_annotations_v3_FileMigrateAnnotation;
extern const upb_MiniTable xds_annotations_v3_MigrateAnnotation_msginit;
extern const upb_MiniTable xds_annotations_v3_FieldMigrateAnnotation_msginit;
extern const upb_MiniTable xds_annotations_v3_FileMigrateAnnotation_msginit;
extern const upb_MiniTable_Extension xds_annotations_v3_message_migrate_ext;
extern const upb_MiniTable_Extension xds_annotations_v3_field_migrate_ext;
extern const upb_MiniTable_Extension xds_annotations_v3_enum_migrate_ext;
extern const upb_MiniTable_Extension xds_annotations_v3_enum_value_migrate_ext;
extern const upb_MiniTable_Extension xds_annotations_v3_file_migrate_ext;
struct google_protobuf_EnumOptions;
struct google_protobuf_EnumValueOptions;
struct google_protobuf_FieldOptions;
struct google_protobuf_FileOptions;
struct google_protobuf_MessageOptions;
extern const upb_MiniTable google_protobuf_EnumOptions_msginit;
extern const upb_MiniTable google_protobuf_EnumValueOptions_msginit;
extern const upb_MiniTable google_protobuf_FieldOptions_msginit;
extern const upb_MiniTable google_protobuf_FileOptions_msginit;
extern const upb_MiniTable google_protobuf_MessageOptions_msginit;



/* xds.annotations.v3.MigrateAnnotation */

UPB_INLINE xds_annotations_v3_MigrateAnnotation* xds_annotations_v3_MigrateAnnotation_new(upb_Arena* arena) {
  return (xds_annotations_v3_MigrateAnnotation*)_upb_Message_New(&xds_annotations_v3_MigrateAnnotation_msginit, arena);
}
UPB_INLINE xds_annotations_v3_MigrateAnnotation* xds_annotations_v3_MigrateAnnotation_parse(const char* buf, size_t size, upb_Arena* arena) {
  xds_annotations_v3_MigrateAnnotation* ret = xds_annotations_v3_MigrateAnnotation_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &xds_annotations_v3_MigrateAnnotation_msginit, NULL, 0, arena) != kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE xds_annotations_v3_MigrateAnnotation* xds_annotations_v3_MigrateAnnotation_parse_ex(const char* buf, size_t size,
                           const upb_ExtensionRegistry* extreg,
                           int options, upb_Arena* arena) {
  xds_annotations_v3_MigrateAnnotation* ret = xds_annotations_v3_MigrateAnnotation_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &xds_annotations_v3_MigrateAnnotation_msginit, extreg, options, arena) !=
      kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE char* xds_annotations_v3_MigrateAnnotation_serialize(const xds_annotations_v3_MigrateAnnotation* msg, upb_Arena* arena, size_t* len) {
  return upb_Encode(msg, &xds_annotations_v3_MigrateAnnotation_msginit, 0, arena, len);
}
UPB_INLINE char* xds_annotations_v3_MigrateAnnotation_serialize_ex(const xds_annotations_v3_MigrateAnnotation* msg, int options,
                                 upb_Arena* arena, size_t* len) {
  return upb_Encode(msg, &xds_annotations_v3_MigrateAnnotation_msginit, options, arena, len);
}
UPB_INLINE void xds_annotations_v3_MigrateAnnotation_clear_rename(const xds_annotations_v3_MigrateAnnotation* msg) {
  *UPB_PTR_AT(msg, UPB_SIZE(0, 0), upb_StringView) = upb_StringView_FromDataAndSize(NULL, 0);
}
UPB_INLINE upb_StringView xds_annotations_v3_MigrateAnnotation_rename(const xds_annotations_v3_MigrateAnnotation* msg) {
  return *UPB_PTR_AT(msg, UPB_SIZE(0, 0), upb_StringView);
}

UPB_INLINE void xds_annotations_v3_MigrateAnnotation_set_rename(xds_annotations_v3_MigrateAnnotation *msg, upb_StringView value) {
  *UPB_PTR_AT(msg, UPB_SIZE(0, 0), upb_StringView) = value;
}

/* xds.annotations.v3.FieldMigrateAnnotation */

UPB_INLINE xds_annotations_v3_FieldMigrateAnnotation* xds_annotations_v3_FieldMigrateAnnotation_new(upb_Arena* arena) {
  return (xds_annotations_v3_FieldMigrateAnnotation*)_upb_Message_New(&xds_annotations_v3_FieldMigrateAnnotation_msginit, arena);
}
UPB_INLINE xds_annotations_v3_FieldMigrateAnnotation* xds_annotations_v3_FieldMigrateAnnotation_parse(const char* buf, size_t size, upb_Arena* arena) {
  xds_annotations_v3_FieldMigrateAnnotation* ret = xds_annotations_v3_FieldMigrateAnnotation_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &xds_annotations_v3_FieldMigrateAnnotation_msginit, NULL, 0, arena) != kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE xds_annotations_v3_FieldMigrateAnnotation* xds_annotations_v3_FieldMigrateAnnotation_parse_ex(const char* buf, size_t size,
                           const upb_ExtensionRegistry* extreg,
                           int options, upb_Arena* arena) {
  xds_annotations_v3_FieldMigrateAnnotation* ret = xds_annotations_v3_FieldMigrateAnnotation_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &xds_annotations_v3_FieldMigrateAnnotation_msginit, extreg, options, arena) !=
      kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE char* xds_annotations_v3_FieldMigrateAnnotation_serialize(const xds_annotations_v3_FieldMigrateAnnotation* msg, upb_Arena* arena, size_t* len) {
  return upb_Encode(msg, &xds_annotations_v3_FieldMigrateAnnotation_msginit, 0, arena, len);
}
UPB_INLINE char* xds_annotations_v3_FieldMigrateAnnotation_serialize_ex(const xds_annotations_v3_FieldMigrateAnnotation* msg, int options,
                                 upb_Arena* arena, size_t* len) {
  return upb_Encode(msg, &xds_annotations_v3_FieldMigrateAnnotation_msginit, options, arena, len);
}
UPB_INLINE void xds_annotations_v3_FieldMigrateAnnotation_clear_rename(const xds_annotations_v3_FieldMigrateAnnotation* msg) {
  *UPB_PTR_AT(msg, UPB_SIZE(0, 0), upb_StringView) = upb_StringView_FromDataAndSize(NULL, 0);
}
UPB_INLINE upb_StringView xds_annotations_v3_FieldMigrateAnnotation_rename(const xds_annotations_v3_FieldMigrateAnnotation* msg) {
  return *UPB_PTR_AT(msg, UPB_SIZE(0, 0), upb_StringView);
}
UPB_INLINE void xds_annotations_v3_FieldMigrateAnnotation_clear_oneof_promotion(const xds_annotations_v3_FieldMigrateAnnotation* msg) {
  *UPB_PTR_AT(msg, UPB_SIZE(8, 16), upb_StringView) = upb_StringView_FromDataAndSize(NULL, 0);
}
UPB_INLINE upb_StringView xds_annotations_v3_FieldMigrateAnnotation_oneof_promotion(const xds_annotations_v3_FieldMigrateAnnotation* msg) {
  return *UPB_PTR_AT(msg, UPB_SIZE(8, 16), upb_StringView);
}

UPB_INLINE void xds_annotations_v3_FieldMigrateAnnotation_set_rename(xds_annotations_v3_FieldMigrateAnnotation *msg, upb_StringView value) {
  *UPB_PTR_AT(msg, UPB_SIZE(0, 0), upb_StringView) = value;
}
UPB_INLINE void xds_annotations_v3_FieldMigrateAnnotation_set_oneof_promotion(xds_annotations_v3_FieldMigrateAnnotation *msg, upb_StringView value) {
  *UPB_PTR_AT(msg, UPB_SIZE(8, 16), upb_StringView) = value;
}

/* xds.annotations.v3.FileMigrateAnnotation */

UPB_INLINE xds_annotations_v3_FileMigrateAnnotation* xds_annotations_v3_FileMigrateAnnotation_new(upb_Arena* arena) {
  return (xds_annotations_v3_FileMigrateAnnotation*)_upb_Message_New(&xds_annotations_v3_FileMigrateAnnotation_msginit, arena);
}
UPB_INLINE xds_annotations_v3_FileMigrateAnnotation* xds_annotations_v3_FileMigrateAnnotation_parse(const char* buf, size_t size, upb_Arena* arena) {
  xds_annotations_v3_FileMigrateAnnotation* ret = xds_annotations_v3_FileMigrateAnnotation_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &xds_annotations_v3_FileMigrateAnnotation_msginit, NULL, 0, arena) != kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE xds_annotations_v3_FileMigrateAnnotation* xds_annotations_v3_FileMigrateAnnotation_parse_ex(const char* buf, size_t size,
                           const upb_ExtensionRegistry* extreg,
                           int options, upb_Arena* arena) {
  xds_annotations_v3_FileMigrateAnnotation* ret = xds_annotations_v3_FileMigrateAnnotation_new(arena);
  if (!ret) return NULL;
  if (upb_Decode(buf, size, ret, &xds_annotations_v3_FileMigrateAnnotation_msginit, extreg, options, arena) !=
      kUpb_DecodeStatus_Ok) {
    return NULL;
  }
  return ret;
}
UPB_INLINE char* xds_annotations_v3_FileMigrateAnnotation_serialize(const xds_annotations_v3_FileMigrateAnnotation* msg, upb_Arena* arena, size_t* len) {
  return upb_Encode(msg, &xds_annotations_v3_FileMigrateAnnotation_msginit, 0, arena, len);
}
UPB_INLINE char* xds_annotations_v3_FileMigrateAnnotation_serialize_ex(const xds_annotations_v3_FileMigrateAnnotation* msg, int options,
                                 upb_Arena* arena, size_t* len) {
  return upb_Encode(msg, &xds_annotations_v3_FileMigrateAnnotation_msginit, options, arena, len);
}
UPB_INLINE void xds_annotations_v3_FileMigrateAnnotation_clear_move_to_package(const xds_annotations_v3_FileMigrateAnnotation* msg) {
  *UPB_PTR_AT(msg, UPB_SIZE(0, 0), upb_StringView) = upb_StringView_FromDataAndSize(NULL, 0);
}
UPB_INLINE upb_StringView xds_annotations_v3_FileMigrateAnnotation_move_to_package(const xds_annotations_v3_FileMigrateAnnotation* msg) {
  return *UPB_PTR_AT(msg, UPB_SIZE(0, 0), upb_StringView);
}

UPB_INLINE void xds_annotations_v3_FileMigrateAnnotation_set_move_to_package(xds_annotations_v3_FileMigrateAnnotation *msg, upb_StringView value) {
  *UPB_PTR_AT(msg, UPB_SIZE(0, 0), upb_StringView) = value;
}

UPB_INLINE bool xds_annotations_v3_has_message_migrate(const struct google_protobuf_MessageOptions* msg) {
  return _upb_Message_Getext(msg, &xds_annotations_v3_message_migrate_ext) != NULL;
}
UPB_INLINE void xds_annotations_v3_clear_message_migrate(struct google_protobuf_MessageOptions* msg) {
  _upb_Message_Clearext(msg, &xds_annotations_v3_message_migrate_ext);
}
UPB_INLINE const xds_annotations_v3_MigrateAnnotation* xds_annotations_v3_message_migrate(const struct google_protobuf_MessageOptions* msg) {
  const upb_Message_Extension* ext = _upb_Message_Getext(msg, &xds_annotations_v3_message_migrate_ext);
  UPB_ASSERT(ext);
  return *UPB_PTR_AT(&ext->data, 0, const xds_annotations_v3_MigrateAnnotation*);
}
UPB_INLINE void xds_annotations_v3_set_message_migrate(struct google_protobuf_MessageOptions* msg, const xds_annotations_v3_MigrateAnnotation* ext, upb_Arena* arena) {
  const upb_Message_Extension* msg_ext =
      _upb_Message_Getorcreateext(msg, &xds_annotations_v3_message_migrate_ext, arena);
  UPB_ASSERT(msg_ext);
  *UPB_PTR_AT(&msg_ext->data, 0, const xds_annotations_v3_MigrateAnnotation*) = ext;
}
UPB_INLINE bool xds_annotations_v3_has_field_migrate(const struct google_protobuf_FieldOptions* msg) {
  return _upb_Message_Getext(msg, &xds_annotations_v3_field_migrate_ext) != NULL;
}
UPB_INLINE void xds_annotations_v3_clear_field_migrate(struct google_protobuf_FieldOptions* msg) {
  _upb_Message_Clearext(msg, &xds_annotations_v3_field_migrate_ext);
}
UPB_INLINE const xds_annotations_v3_FieldMigrateAnnotation* xds_annotations_v3_field_migrate(const struct google_protobuf_FieldOptions* msg) {
  const upb_Message_Extension* ext = _upb_Message_Getext(msg, &xds_annotations_v3_field_migrate_ext);
  UPB_ASSERT(ext);
  return *UPB_PTR_AT(&ext->data, 0, const xds_annotations_v3_FieldMigrateAnnotation*);
}
UPB_INLINE void xds_annotations_v3_set_field_migrate(struct google_protobuf_FieldOptions* msg, const xds_annotations_v3_FieldMigrateAnnotation* ext, upb_Arena* arena) {
  const upb_Message_Extension* msg_ext =
      _upb_Message_Getorcreateext(msg, &xds_annotations_v3_field_migrate_ext, arena);
  UPB_ASSERT(msg_ext);
  *UPB_PTR_AT(&msg_ext->data, 0, const xds_annotations_v3_FieldMigrateAnnotation*) = ext;
}
UPB_INLINE bool xds_annotations_v3_has_enum_migrate(const struct google_protobuf_EnumOptions* msg) {
  return _upb_Message_Getext(msg, &xds_annotations_v3_enum_migrate_ext) != NULL;
}
UPB_INLINE void xds_annotations_v3_clear_enum_migrate(struct google_protobuf_EnumOptions* msg) {
  _upb_Message_Clearext(msg, &xds_annotations_v3_enum_migrate_ext);
}
UPB_INLINE const xds_annotations_v3_MigrateAnnotation* xds_annotations_v3_enum_migrate(const struct google_protobuf_EnumOptions* msg) {
  const upb_Message_Extension* ext = _upb_Message_Getext(msg, &xds_annotations_v3_enum_migrate_ext);
  UPB_ASSERT(ext);
  return *UPB_PTR_AT(&ext->data, 0, const xds_annotations_v3_MigrateAnnotation*);
}
UPB_INLINE void xds_annotations_v3_set_enum_migrate(struct google_protobuf_EnumOptions* msg, const xds_annotations_v3_MigrateAnnotation* ext, upb_Arena* arena) {
  const upb_Message_Extension* msg_ext =
      _upb_Message_Getorcreateext(msg, &xds_annotations_v3_enum_migrate_ext, arena);
  UPB_ASSERT(msg_ext);
  *UPB_PTR_AT(&msg_ext->data, 0, const xds_annotations_v3_MigrateAnnotation*) = ext;
}
UPB_INLINE bool xds_annotations_v3_has_enum_value_migrate(const struct google_protobuf_EnumValueOptions* msg) {
  return _upb_Message_Getext(msg, &xds_annotations_v3_enum_value_migrate_ext) != NULL;
}
UPB_INLINE void xds_annotations_v3_clear_enum_value_migrate(struct google_protobuf_EnumValueOptions* msg) {
  _upb_Message_Clearext(msg, &xds_annotations_v3_enum_value_migrate_ext);
}
UPB_INLINE const xds_annotations_v3_MigrateAnnotation* xds_annotations_v3_enum_value_migrate(const struct google_protobuf_EnumValueOptions* msg) {
  const upb_Message_Extension* ext = _upb_Message_Getext(msg, &xds_annotations_v3_enum_value_migrate_ext);
  UPB_ASSERT(ext);
  return *UPB_PTR_AT(&ext->data, 0, const xds_annotations_v3_MigrateAnnotation*);
}
UPB_INLINE void xds_annotations_v3_set_enum_value_migrate(struct google_protobuf_EnumValueOptions* msg, const xds_annotations_v3_MigrateAnnotation* ext, upb_Arena* arena) {
  const upb_Message_Extension* msg_ext =
      _upb_Message_Getorcreateext(msg, &xds_annotations_v3_enum_value_migrate_ext, arena);
  UPB_ASSERT(msg_ext);
  *UPB_PTR_AT(&msg_ext->data, 0, const xds_annotations_v3_MigrateAnnotation*) = ext;
}
UPB_INLINE bool xds_annotations_v3_has_file_migrate(const struct google_protobuf_FileOptions* msg) {
  return _upb_Message_Getext(msg, &xds_annotations_v3_file_migrate_ext) != NULL;
}
UPB_INLINE void xds_annotations_v3_clear_file_migrate(struct google_protobuf_FileOptions* msg) {
  _upb_Message_Clearext(msg, &xds_annotations_v3_file_migrate_ext);
}
UPB_INLINE const xds_annotations_v3_FileMigrateAnnotation* xds_annotations_v3_file_migrate(const struct google_protobuf_FileOptions* msg) {
  const upb_Message_Extension* ext = _upb_Message_Getext(msg, &xds_annotations_v3_file_migrate_ext);
  UPB_ASSERT(ext);
  return *UPB_PTR_AT(&ext->data, 0, const xds_annotations_v3_FileMigrateAnnotation*);
}
UPB_INLINE void xds_annotations_v3_set_file_migrate(struct google_protobuf_FileOptions* msg, const xds_annotations_v3_FileMigrateAnnotation* ext, upb_Arena* arena) {
  const upb_Message_Extension* msg_ext =
      _upb_Message_Getorcreateext(msg, &xds_annotations_v3_file_migrate_ext, arena);
  UPB_ASSERT(msg_ext);
  *UPB_PTR_AT(&msg_ext->data, 0, const xds_annotations_v3_FileMigrateAnnotation*) = ext;
}
extern const upb_MiniTable_File xds_annotations_v3_migrate_proto_upb_file_layout;

#ifdef __cplusplus
}  /* extern "C" */
#endif

#include "upb/port_undef.inc"

#endif  /* XDS_ANNOTATIONS_V3_MIGRATE_PROTO_UPB_H_ */
