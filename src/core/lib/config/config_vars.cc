/*
 * Copyright 2022 gRPC authors.
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
 */

/*
 * Automatically generated by tools/codegen/core/gen_config_vars.py
 */

#include <grpc/support/port_platform.h>

#include "src/core/lib/config/config_vars.h"

#include "absl/flags/flag.h"

#include "src/core/lib/config/load_config.h"

#ifndef GPR_DEFAULT_LOG_VERBOSITY_STRING
#define GPR_DEFAULT_LOG_VERBOSITY_STRING "ERROR"
#endif  // !GPR_DEFAULT_LOG_VERBOSITY_STRING

#ifdef GRPC_ENABLE_FORK_SUPPORT
#define GRPC_ENABLE_FORK_SUPPORT_DEFAULT true
#else
#define GRPC_ENABLE_FORK_SUPPORT_DEFAULT false
#endif  // GRPC_ENABLE_FORK_SUPPORT

namespace {
const char* const description_experiments =
    "A comma separated list of currently active experiments. Experiments may "
    "be prefixed with a '-' to disable them.";
const char* const description_client_channel_backup_poll_interval_ms =
    "Declares the interval in ms between two backup polls on client channels. "
    "These polls are run in the timer thread so that gRPC can process "
    "connection failures while there is no active polling thread. They help "
    "reconnect disconnected client channels (mostly due to idleness), so that "
    "the next RPC on this channel won't fail. Set to 0 to turn off the backup "
    "polls.";
const char* const description_dns_resolver =
    "Declares which DNS resolver to use. The default is ares if gRPC is built "
    "with c-ares support. Otherwise, the value of this environment variable is "
    "ignored.";
const char* const description_trace =
    "A comma separated list of tracers that provide additional insight into "
    "how gRPC C core is processing requests via debug logs.";
const char* const description_verbosity = "Default gRPC logging verbosity";
const char* const description_stacktrace_minloglevel =
    "Messages logged at the same or higher level than this will print "
    "stacktrace";
const char* const description_enable_fork_support = "Enable fork support";
const char* const description_poll_strategy =
    "Declares which polling engines to try when starting gRPC. This is a "
    "comma-separated list of engines, which are tried in priority order first "
    "-> last.";
const char* const description_abort_on_leaks =
    "A debugging aid to cause a call to abort() when gRPC objects are leaked "
    "past grpc_shutdown()";
const char* const description_system_ssl_roots_dir =
    "Custom directory to SSL Roots";
const char* const description_default_ssl_roots_file_path =
    "Path to the default SSL roots file.";
const char* const description_not_use_system_ssl_roots =
    "Disable loading system root certificates.";
const char* const description_ssl_cipher_suites =
    "A colon separated list of cipher suites to use with OpenSSL";
const char* const default_experiments = "";
const char* const default_dns_resolver = "";
const char* const default_trace = "";
const char* const default_verbosity = "GPR_DEFAULT_LOG_VERBOSITY_STRING";
const char* const default_stacktrace_minloglevel = "";
const char* const default_poll_strategy = "all";
const char* const default_system_ssl_roots_dir = "";
const char* const default_default_ssl_roots_file_path = "";
const char* const default_ssl_cipher_suites =
    "TLS_AES_128_GCM_SHA256:TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_"
    "SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-"
    "RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384";
}  // namespace
ABSL_FLAG(absl::optional<std::string>, grpc_experiments, absl::nullopt,
          description_experiments);
ABSL_FLAG(absl::optional<int32_t>, grpc_client_channel_backup_poll_interval_ms,
          absl::nullopt, description_client_channel_backup_poll_interval_ms);
ABSL_FLAG(absl::optional<std::string>, grpc_dns_resolver, absl::nullopt,
          description_dns_resolver);
ABSL_FLAG(absl::optional<std::string>, grpc_trace, absl::nullopt,
          description_trace);
ABSL_FLAG(absl::optional<std::string>, grpc_verbosity, absl::nullopt,
          description_verbosity);
ABSL_FLAG(absl::optional<std::string>, grpc_stacktrace_minloglevel,
          absl::nullopt, description_stacktrace_minloglevel);
ABSL_FLAG(absl::optional<bool>, grpc_enable_fork_support, absl::nullopt,
          description_enable_fork_support);
ABSL_FLAG(absl::optional<std::string>, grpc_poll_strategy, absl::nullopt,
          description_poll_strategy);
ABSL_FLAG(absl::optional<bool>, grpc_abort_on_leaks, absl::nullopt,
          description_abort_on_leaks);
ABSL_FLAG(absl::optional<std::string>, grpc_system_ssl_roots_dir, absl::nullopt,
          description_system_ssl_roots_dir);
ABSL_FLAG(absl::optional<std::string>, grpc_default_ssl_roots_file_path,
          absl::nullopt, description_default_ssl_roots_file_path);
ABSL_FLAG(absl::optional<bool>, grpc_not_use_system_ssl_roots, absl::nullopt,
          description_not_use_system_ssl_roots);
ABSL_FLAG(absl::optional<std::string>, grpc_ssl_cipher_suites, absl::nullopt,
          description_ssl_cipher_suites);

namespace grpc_core {

ConfigVars::ConfigVars(const Overrides& overrides)
    : client_channel_backup_poll_interval_ms_(
          LoadConfig(FLAGS_grpc_client_channel_backup_poll_interval_ms,
                     overrides.client_channel_backup_poll_interval_ms, 5000)),
      enable_fork_support_(LoadConfig(FLAGS_grpc_enable_fork_support,
                                      overrides.enable_fork_support,
                                      GRPC_ENABLE_FORK_SUPPORT_DEFAULT)),
      abort_on_leaks_(LoadConfig(FLAGS_grpc_abort_on_leaks,
                                 overrides.abort_on_leaks, false)),
      not_use_system_ssl_roots_(LoadConfig(FLAGS_grpc_not_use_system_ssl_roots,
                                           overrides.not_use_system_ssl_roots,
                                           false)),
      experiments_(LoadConfig(FLAGS_grpc_experiments, overrides.experiments,
                              default_experiments)),
      dns_resolver_(LoadConfig(FLAGS_grpc_dns_resolver, overrides.dns_resolver,
                               default_dns_resolver)),
      trace_(LoadConfig(FLAGS_grpc_trace, overrides.trace, default_trace)),
      verbosity_(LoadConfig(FLAGS_grpc_verbosity, overrides.verbosity,
                            default_verbosity)),
      stacktrace_minloglevel_(LoadConfig(FLAGS_grpc_stacktrace_minloglevel,
                                         overrides.stacktrace_minloglevel,
                                         default_stacktrace_minloglevel)),
      poll_strategy_(LoadConfig(FLAGS_grpc_poll_strategy,
                                overrides.poll_strategy,
                                default_poll_strategy)),
      system_ssl_roots_dir_(LoadConfig(FLAGS_grpc_system_ssl_roots_dir,
                                       overrides.system_ssl_roots_dir,
                                       default_system_ssl_roots_dir)),
      default_ssl_roots_file_path_(
          LoadConfig(FLAGS_grpc_default_ssl_roots_file_path,
                     overrides.default_ssl_roots_file_path,
                     default_default_ssl_roots_file_path)),
      ssl_cipher_suites_(LoadConfig(FLAGS_grpc_ssl_cipher_suites,
                                    overrides.ssl_cipher_suites,
                                    default_ssl_cipher_suites)) {}

}  // namespace grpc_core
