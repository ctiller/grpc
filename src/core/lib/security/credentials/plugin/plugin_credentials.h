/*
 *
 * Copyright 2016 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef GRPC_CORE_LIB_SECURITY_CREDENTIALS_PLUGIN_PLUGIN_CREDENTIALS_H
#define GRPC_CORE_LIB_SECURITY_CREDENTIALS_PLUGIN_PLUGIN_CREDENTIALS_H

#include <grpc/support/port_platform.h>

#include "src/core/lib/security/credentials/credentials.h"

extern grpc_core::TraceFlag grpc_plugin_credentials_trace;

// This type is forward declared as a C struct and we cannot define it as a
// class. Otherwise, compiler will complain about type mismatch due to
// -Wmismatched-tags.
struct grpc_plugin_credentials final : public grpc_call_credentials {
 public:
  struct pending_request : public grpc_core::RefCounted<pending_request> {
    ~pending_request() override {
      grpc_auth_metadata_context_reset(&context);
      for (size_t i = 0; i < metadata.size(); i++) {
        grpc_slice_unref_internal(metadata[i].key);
        grpc_slice_unref_internal(metadata[i].value);
      }
    }
    std::atomic<bool> ready;
    grpc_core::Waker waker;
    struct grpc_plugin_credentials* creds;
    grpc_core::ClientInitialMetadata md;
    grpc_core::RefCountedPtr<grpc_call_credentials> call_creds;
    grpc_auth_metadata_context context;
    // final status
    absl::InlinedVector<grpc_metadata, 2> metadata;
    std::string error_details;
    grpc_status_code status;
  };

  explicit grpc_plugin_credentials(grpc_metadata_credentials_plugin plugin,
                                   grpc_security_level min_security_level);
  ~grpc_plugin_credentials() override;

  grpc_core::ArenaPromise<absl::StatusOr<grpc_core::ClientInitialMetadata>>
  GetRequestMetadata(
      grpc_core::ClientInitialMetadata initial_metadata,
      grpc_core::AuthMetadataContext* auth_metadata_context) override;

  std::string debug_string() override;

 private:
  grpc_metadata_credentials_plugin plugin_;
};

#endif /* GRPC_CORE_LIB_SECURITY_CREDENTIALS_PLUGIN_PLUGIN_CREDENTIALS_H */
