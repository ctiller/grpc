# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: envoy/extensions/outlier_detection_monitors/consecutive_errors/v3/consecutive_errors.proto
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from envoy.extensions.outlier_detection_monitors.common.v3 import error_types_pb2 as envoy_dot_extensions_dot_outlier__detection__monitors_dot_common_dot_v3_dot_error__types__pb2
from google.protobuf import wrappers_pb2 as google_dot_protobuf_dot_wrappers__pb2
from udpa.annotations import status_pb2 as udpa_dot_annotations_dot_status__pb2
from validate import validate_pb2 as validate_dot_validate__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\nZenvoy/extensions/outlier_detection_monitors/consecutive_errors/v3/consecutive_errors.proto\x12\x41\x65nvoy.extensions.outlier_detection_monitors.consecutive_errors.v3\x1aGenvoy/extensions/outlier_detection_monitors/common/v3/error_types.proto\x1a\x1egoogle/protobuf/wrappers.proto\x1a\x1dudpa/annotations/status.proto\x1a\x17validate/validate.proto\"\xf1\x01\n\x11\x43onsecutiveErrors\x12\x0c\n\x04name\x18\x01 \x01(\t\x12\x38\n\tthreshold\x18\x02 \x01(\x0b\x32\x1c.google.protobuf.UInt32ValueB\x07\xfa\x42\x04*\x02\x18\x64\x12\x38\n\tenforcing\x18\x03 \x01(\x0b\x32\x1c.google.protobuf.UInt32ValueB\x07\xfa\x42\x04*\x02\x18\x64\x12Z\n\rerror_buckets\x18\x04 \x01(\x0b\x32\x43.envoy.extensions.outlier_detection_monitors.common.v3.ErrorBucketsB\xf2\x01\nOio.envoyproxy.envoy.extensions.outlier_detection_monitors.consecutive_errors.v3B\x16\x43onsecutiveErrorsProtoP\x01Z}github.com/envoyproxy/go-control-plane/envoy/extensions/outlier_detection_monitors/consecutive_errors/v3;consecutive_errorsv3\xba\x80\xc8\xd1\x06\x02\x10\x02\x62\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'envoy.extensions.outlier_detection_monitors.consecutive_errors.v3.consecutive_errors_pb2', _globals)
if _descriptor._USE_C_DESCRIPTORS == False:
  DESCRIPTOR._options = None
  DESCRIPTOR._serialized_options = b'\nOio.envoyproxy.envoy.extensions.outlier_detection_monitors.consecutive_errors.v3B\026ConsecutiveErrorsProtoP\001Z}github.com/envoyproxy/go-control-plane/envoy/extensions/outlier_detection_monitors/consecutive_errors/v3;consecutive_errorsv3\272\200\310\321\006\002\020\002'
  _CONSECUTIVEERRORS.fields_by_name['threshold']._options = None
  _CONSECUTIVEERRORS.fields_by_name['threshold']._serialized_options = b'\372B\004*\002\030d'
  _CONSECUTIVEERRORS.fields_by_name['enforcing']._options = None
  _CONSECUTIVEERRORS.fields_by_name['enforcing']._serialized_options = b'\372B\004*\002\030d'
  _globals['_CONSECUTIVEERRORS']._serialized_start=323
  _globals['_CONSECUTIVEERRORS']._serialized_end=564
# @@protoc_insertion_point(module_scope)
