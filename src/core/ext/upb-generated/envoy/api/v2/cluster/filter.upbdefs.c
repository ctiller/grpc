/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/api/v2/cluster/filter.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"
#include "envoy/api/v2/cluster/filter.upbdefs.h"

extern upb_def_init google_protobuf_any_proto_upbdefinit;
extern upb_def_init udpa_annotations_migrate_proto_upbdefinit;
extern upb_def_init udpa_annotations_status_proto_upbdefinit;
extern upb_def_init validate_validate_proto_upbdefinit;
extern const upb_msglayout envoy_api_v2_cluster_Filter_msginit;

static const upb_msglayout *layouts[1] = {
  &envoy_api_v2_cluster_Filter_msginit,
};

static const char descriptor[419] = {'\n', '!', 'e', 'n', 'v', 'o', 'y', '/', 'a', 'p', 'i', '/', 'v', '2', '/', 'c', 'l', 'u', 's', 't', 'e', 'r', '/', 'f', 'i', 
'l', 't', 'e', 'r', '.', 'p', 'r', 'o', 't', 'o', '\022', '\024', 'e', 'n', 'v', 'o', 'y', '.', 'a', 'p', 'i', '.', 'v', '2', '.', 
'c', 'l', 'u', 's', 't', 'e', 'r', '\032', '\031', 'g', 'o', 'o', 'g', 'l', 'e', '/', 'p', 'r', 'o', 't', 'o', 'b', 'u', 'f', '/', 
'a', 'n', 'y', '.', 'p', 'r', 'o', 't', 'o', '\032', '\036', 'u', 'd', 'p', 'a', '/', 'a', 'n', 'n', 'o', 't', 'a', 't', 'i', 'o', 
'n', 's', '/', 'm', 'i', 'g', 'r', 'a', 't', 'e', '.', 'p', 'r', 'o', 't', 'o', '\032', '\035', 'u', 'd', 'p', 'a', '/', 'a', 'n', 
'n', 'o', 't', 'a', 't', 'i', 'o', 'n', 's', '/', 's', 't', 'a', 't', 'u', 's', '.', 'p', 'r', 'o', 't', 'o', '\032', '\027', 'v', 
'a', 'l', 'i', 'd', 'a', 't', 'e', '/', 'v', 'a', 'l', 'i', 'd', 'a', 't', 'e', '.', 'p', 'r', 'o', 't', 'o', '\"', '^', '\n', 
'\006', 'F', 'i', 'l', 't', 'e', 'r', '\022', '\033', '\n', '\004', 'n', 'a', 'm', 'e', '\030', '\001', ' ', '\001', '(', '\t', 'B', '\007', '\372', 'B', 
'\004', 'r', '\002', ' ', '\001', 'R', '\004', 'n', 'a', 'm', 'e', '\022', '7', '\n', '\014', 't', 'y', 'p', 'e', 'd', '_', 'c', 'o', 'n', 'f', 
'i', 'g', '\030', '\002', ' ', '\001', '(', '\013', '2', '\024', '.', 'g', 'o', 'o', 'g', 'l', 'e', '.', 'p', 'r', 'o', 't', 'o', 'b', 'u', 
'f', '.', 'A', 'n', 'y', 'R', '\013', 't', 'y', 'p', 'e', 'd', 'C', 'o', 'n', 'f', 'i', 'g', 'B', '\214', '\001', '\n', '\"', 'i', 'o', 
'.', 'e', 'n', 'v', 'o', 'y', 'p', 'r', 'o', 'x', 'y', '.', 'e', 'n', 'v', 'o', 'y', '.', 'a', 'p', 'i', '.', 'v', '2', '.', 
'c', 'l', 'u', 's', 't', 'e', 'r', 'B', '\013', 'F', 'i', 'l', 't', 'e', 'r', 'P', 'r', 'o', 't', 'o', 'P', '\001', '\252', '\002', '\026', 
'E', 'n', 'v', 'o', 'y', '.', 'A', 'p', 'i', '.', 'V', '2', '.', 'C', 'l', 'u', 's', 't', 'e', 'r', 'N', 'S', '\352', '\002', '\026', 
'E', 'n', 'v', 'o', 'y', '.', 'A', 'p', 'i', '.', 'V', '2', '.', 'C', 'l', 'u', 's', 't', 'e', 'r', 'N', 'S', '\362', '\230', '\376', 
'\217', '\005', '\031', '\022', '\027', 'e', 'n', 'v', 'o', 'y', '.', 'c', 'o', 'n', 'f', 'i', 'g', '.', 'c', 'l', 'u', 's', 't', 'e', 'r', 
'.', 'v', '3', '\272', '\200', '\310', '\321', '\006', '\002', '\020', '\001', 'b', '\006', 'p', 'r', 'o', 't', 'o', '3', 
};

static upb_def_init *deps[5] = {
  &google_protobuf_any_proto_upbdefinit,
  &udpa_annotations_migrate_proto_upbdefinit,
  &udpa_annotations_status_proto_upbdefinit,
  &validate_validate_proto_upbdefinit,
  NULL
};

upb_def_init envoy_api_v2_cluster_filter_proto_upbdefinit = {
  deps,
  layouts,
  "envoy/api/v2/cluster/filter.proto",
  UPB_STRVIEW_INIT(descriptor, 419)
};
