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
  explicit grpc_plugin_credentials(grpc_metadata_credentials_plugin plugin,
                                   grpc_security_level min_security_level);
  ~grpc_plugin_credentials() override;

  grpc_core::ArenaPromise<absl::StatusOr<grpc_core::ClientInitialMetadata>>
  GetRequestMetadata(
      grpc_core::ClientInitialMetadata initial_metadata,
      grpc_core::AuthMetadataContext* auth_metadata_context) override;

  std::string debug_string() override;

 private:
  class PendingRequest : public grpc_core::RefCounted<PendingRequest> {
   public:
    PendingRequest(grpc_plugin_credentials* creds,
                   grpc_core::AuthMetadataContext* auth_metadata_context,
                   grpc_core::ClientInitialMetadata initial_metadata)
        : call_creds_(creds->Ref()),
          context_(auth_metadata_context->MakeLegacyContext(initial_metadata)),
          md_(std::move(initial_metadata)) {}

    ~PendingRequest() override {
      grpc_auth_metadata_context_reset(&context_);
      for (size_t i = 0; i < metadata_.size(); i++) {
        grpc_slice_unref_internal(metadata_[i].key);
        grpc_slice_unref_internal(metadata_[i].value);
      }
    }

    absl::StatusOr<grpc_core::ClientInitialMetadata> ProcessPluginResult(
        const grpc_metadata* md, size_t num_md, grpc_status_code status,
        const char* error_details);

    grpc_core::Poll<absl::StatusOr<grpc_core::ClientInitialMetadata>>
    PollAsyncResult();

    static void RequestMetadataReady(void* request, const grpc_metadata* md,
                                     size_t num_md, grpc_status_code status,
                                     const char* error_details);

    grpc_auth_metadata_context context() const { return context_; }
    grpc_plugin_credentials* creds() const { return call_creds_.get(); }

   private:
    std::atomic<bool> ready_{false};
    grpc_core::Waker waker_{
        grpc_core::Activity::current()->MakeNonOwningWaker()};
    grpc_core::RefCountedPtr<grpc_plugin_credentials> call_creds_;
    grpc_auth_metadata_context context_;
    grpc_core::ClientInitialMetadata md_;
    // final status
    absl::InlinedVector<grpc_metadata, 2> metadata_;
    std::string error_details_;
    grpc_status_code status_;
  };

  grpc_metadata_credentials_plugin plugin_;
};

#endif /* GRPC_CORE_LIB_SECURITY_CREDENTIALS_PLUGIN_PLUGIN_CREDENTIALS_H */
