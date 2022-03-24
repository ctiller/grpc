//
// Copyright 2022 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef GRPCPP_EXT_ORCA_SERVICE_H
#define GRPCPP_EXT_ORCA_SERVICE_H

#include <map>
#include <string>

#include <grpcpp/impl/codegen/async_generic_service.h>
#include <grpcpp/impl/codegen/sync.h>
#include <grpcpp/server_builder.h>

namespace grpc {
namespace experimental {

// RPC service implementation for supplying out-of-band backend
// utilization metrics to clients.
class OrcaService : public CallbackGenericService {
 public:
  struct Options {
    // Minimum report interval.  If a client requests an interval lower
    // than this value, this value will be used instead.
    int min_report_duration_ms = 30 * 1000;

    Options() = default;
    Options& set_min_report_duration_ms(int ms) {
      min_report_duration_ms = ms;
      return *this;
    }
  };

  explicit OrcaService(Options options)
      : min_report_duration_ms_(options.min_report_duration_ms) {}

  void Register(ServerBuilder* builder);

  // Sets or removes the CPU utilization value to be reported to clients.
  void SetCpuUtilization(double cpu_utilization);
  void DeleteCpuUtilization();

  // Sets of removes the memory utilization value to be reported to clients.
  void SetMemoryUtilization(double memory_utilization);
  void DeleteMemoryUtilization();

  // Sets or removed named utilization values to be reported to clients.
  void SetNamedUtilization(std::string name, double utilization);
  void DeleteNamedUtilization(const std::string& name);
  void SetAllNamedUtilization(std::map<std::string, double> named_utilization);

 private:
  class Reactor;

  ServerGenericBidiReactor* CreateReactor(
      GenericCallbackServerContext* /*ctx*/) override;

  const int min_report_duration_ms_;

  grpc::internal::Mutex mu_;
  double cpu_utilization_ ABSL_GUARDED_BY(&mu_) = -1;
  double memory_utilization_ ABSL_GUARDED_BY(&mu_) = -1;
  std::map<std::string, double> named_utilization_ ABSL_GUARDED_BY(&mu_);
};

}  // namespace experimental
}  // namespace grpc

#endif  // GRPCPP_EXT_ORCA_SERVICE_H
