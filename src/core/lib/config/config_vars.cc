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

#include <vector>

#include "absl/flags/flag.h"

#include "src/core/lib/config/config_from_environment.h"

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
ABSL_FLAG(std::string, grpc_experiments,
          grpc_core::LoadStringFromEnv("experiments", default_experiments),
          description_experiments);
ABSL_FLAG(int32_t, grpc_client_channel_backup_poll_interval_ms,
          grpc_core::LoadIntFromEnv("client_channel_backup_poll_interval_ms",
                                    5000),
          description_client_channel_backup_poll_interval_ms);
ABSL_FLAG(std::string, grpc_dns_resolver,
          grpc_core::LoadStringFromEnv("dns_resolver", default_dns_resolver),
          description_dns_resolver);
ABSL_FLAG(std::string, grpc_trace,
          grpc_core::LoadStringFromEnv("trace", default_trace),
          description_trace);
ABSL_FLAG(std::string, grpc_verbosity,
          grpc_core::LoadStringFromEnv("verbosity", default_verbosity),
          description_verbosity);
ABSL_FLAG(std::string, grpc_stacktrace_minloglevel,
          grpc_core::LoadStringFromEnv("stacktrace_minloglevel",
                                       default_stacktrace_minloglevel),
          description_stacktrace_minloglevel);
ABSL_FLAG(bool, grpc_enable_fork_support,
          grpc_core::LoadBoolFromEnv("enable_fork_support", true),
          description_enable_fork_support);
ABSL_FLAG(std::string, grpc_poll_strategy,
          grpc_core::LoadStringFromEnv("poll_strategy", default_poll_strategy),
          description_poll_strategy);
ABSL_FLAG(bool, grpc_abort_on_leaks,
          grpc_core::LoadBoolFromEnv("abort_on_leaks", false),
          description_abort_on_leaks);
ABSL_FLAG(std::string, grpc_system_ssl_roots_dir,
          grpc_core::LoadStringFromEnv("system_ssl_roots_dir",
                                       default_system_ssl_roots_dir),
          description_system_ssl_roots_dir);
ABSL_FLAG(std::string, grpc_default_ssl_roots_file_path,
          grpc_core::LoadStringFromEnv("default_ssl_roots_file_path",
                                       default_default_ssl_roots_file_path),
          description_default_ssl_roots_file_path);
ABSL_FLAG(bool, grpc_not_use_system_ssl_roots,
          grpc_core::LoadBoolFromEnv("not_use_system_ssl_roots", false),
          description_not_use_system_ssl_roots);
ABSL_FLAG(std::string, grpc_ssl_cipher_suites,
          grpc_core::LoadStringFromEnv("ssl_cipher_suites",
                                       default_ssl_cipher_suites),
          description_ssl_cipher_suites);

namespace grpc_core {

ConfigVars::ConfigVars()
    : client_channel_backup_poll_interval_ms_(
          absl::GetFlag(FLAGS_grpc_client_channel_backup_poll_interval_ms)),
      enable_fork_support_(absl::GetFlag(FLAGS_grpc_enable_fork_support)),
      abort_on_leaks_(absl::GetFlag(FLAGS_grpc_abort_on_leaks)),
      not_use_system_ssl_roots_(
          absl::GetFlag(FLAGS_grpc_not_use_system_ssl_roots)),
      experiments_(absl::GetFlag(FLAGS_grpc_experiments)),
      dns_resolver_(absl::GetFlag(FLAGS_grpc_dns_resolver)),
      trace_(absl::GetFlag(FLAGS_grpc_trace)),
      verbosity_(absl::GetFlag(FLAGS_grpc_verbosity)),
      stacktrace_minloglevel_(absl::GetFlag(FLAGS_grpc_stacktrace_minloglevel)),
      poll_strategy_(absl::GetFlag(FLAGS_grpc_poll_strategy)),
      system_ssl_roots_dir_(absl::GetFlag(FLAGS_grpc_system_ssl_roots_dir)),
      default_ssl_roots_file_path_(
          absl::GetFlag(FLAGS_grpc_default_ssl_roots_file_path)),
      ssl_cipher_suites_(absl::GetFlag(FLAGS_grpc_ssl_cipher_suites)) {}

absl::Span<const ConfigVarMetadata> ConfigVars::metadata() {
  static const auto* metadata = new std::vector<ConfigVarMetadata>{
      {
          "experiments",
          description_experiments,
          ConfigVarMetadata::String{default_experiments,
                                    &ConfigVars::Experiments},
      },
      {
          "client_channel_backup_poll_interval_ms",
          description_client_channel_backup_poll_interval_ms,
          ConfigVarMetadata::Int{
              5000, &ConfigVars::ClientChannelBackupPollIntervalMs},
      },
      {
          "dns_resolver",
          description_dns_resolver,
          ConfigVarMetadata::String{default_dns_resolver,
                                    &ConfigVars::DnsResolver},
      },
      {
          "trace",
          description_trace,
          ConfigVarMetadata::String{default_trace, &ConfigVars::Trace},
      },
      {
          "verbosity",
          description_verbosity,
          ConfigVarMetadata::String{default_verbosity, &ConfigVars::Verbosity},
      },
      {
          "stacktrace_minloglevel",
          description_stacktrace_minloglevel,
          ConfigVarMetadata::String{default_stacktrace_minloglevel,
                                    &ConfigVars::StacktraceMinloglevel},
      },
      {
          "enable_fork_support",
          description_enable_fork_support,
          ConfigVarMetadata::Bool{true, &ConfigVars::EnableForkSupport},
      },
      {
          "poll_strategy",
          description_poll_strategy,
          ConfigVarMetadata::String{default_poll_strategy,
                                    &ConfigVars::PollStrategy},
      },
      {
          "abort_on_leaks",
          description_abort_on_leaks,
          ConfigVarMetadata::Bool{false, &ConfigVars::AbortOnLeaks},
      },
      {
          "system_ssl_roots_dir",
          description_system_ssl_roots_dir,
          ConfigVarMetadata::String{default_system_ssl_roots_dir,
                                    &ConfigVars::SystemSslRootsDir},
      },
      {
          "default_ssl_roots_file_path",
          description_default_ssl_roots_file_path,
          ConfigVarMetadata::String{default_default_ssl_roots_file_path,
                                    &ConfigVars::DefaultSslRootsFilePath},
      },
      {
          "not_use_system_ssl_roots",
          description_not_use_system_ssl_roots,
          ConfigVarMetadata::Bool{false, &ConfigVars::NotUseSystemSslRoots},
      },
      {
          "ssl_cipher_suites",
          description_ssl_cipher_suites,
          ConfigVarMetadata::String{default_ssl_cipher_suites,
                                    &ConfigVars::SslCipherSuites},
      },
  };
  return *metadata;
}

}  // namespace grpc_core
