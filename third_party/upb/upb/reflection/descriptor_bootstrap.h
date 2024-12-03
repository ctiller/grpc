#ifndef THIRD_PARTY_UPB_UPB_REFLECTION_DESCRIPTOR_BOOTSTRAP_H_
#define THIRD_PARTY_UPB_UPB_REFLECTION_DESCRIPTOR_BOOTSTRAP_H_

// IWYU pragma: begin_exports

#if defined(UPB_BOOTSTRAP_STAGE) && UPB_BOOTSTRAP_STAGE == 0
// This header is checked in.
#include "upb/reflection/stage0/google/protobuf/descriptor.upb.h"
#elif UPB_BOOTSTRAP_STAGE == 1
// This header is generated at build time by the bootstrapping process.
#include "upb/reflection/stage1/google/protobuf/descriptor.upb.h"
#else
// This is the normal header, generated by upb_c_proto_library().
#include "google/protobuf/descriptor.upb.h"
#endif

// IWYU pragma: end_exports

#endif  // THIRD_PARTY_UPB_UPB_REFLECTION_DESCRIPTOR_BOOTSTRAP_H_
