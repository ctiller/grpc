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

#include "google/protobuf/duration.upb.h"
#include "upb/upb.hpp"
#include "xds/data/orca/v3/orca_load_report.upb.h"
#include "xds/service/orca/v3/orca.upb.h"

#include <grpcpp/ext/orca_service.h>

#include "src/core/lib/gprpp/time.h"
#include "src/core/lib/iomgr/timer.h"

namespace grpc {
namespace experimental {

//
// OrcaService::Reactor
//

class OrcaService::Reactor : public ServerGenericBidiReactor,
                             public grpc_core::RefCounted<Reactor> {
 public:
  explicit Reactor(OrcaService* service) : service_(service) {
    GRPC_CLOSURE_INIT(&on_timer_, OnTimer, this, nullptr);
    StartRead(&request_);
  }

  void OnReadDone(bool ok) override {
    if (!ok) {
      Finish(Status(StatusCode::UNKNOWN, "read failed"));
      return;
    }
    // Get slice from request.
    Slice slice;
    request_.DumpToSingleSlice(&slice);
    request_.Clear();
    // Parse request proto.
    upb::Arena arena;
    xds_service_orca_v3_OrcaLoadReportRequest* request =
        xds_service_orca_v3_OrcaLoadReportRequest_parse(
            reinterpret_cast<const char*>(slice.begin()), slice.size(),
            arena.ptr());
    if (request == nullptr) {
      Finish(Status(StatusCode::UNKNOWN, "could not parse request proto"));
      return;
    }
    const auto* duration_proto =
        xds_service_orca_v3_OrcaLoadReportRequest_report_interval(request);
    if (duration_proto != nullptr) {
      report_interval_ = grpc_core::Duration::FromSecondsAndNanoseconds(
          google_protobuf_Duration_seconds(duration_proto),
          google_protobuf_Duration_nanos(duration_proto));
    }
    auto min_interval =
        grpc_core::Duration::Milliseconds(service_->min_report_duration_ms_);
    if (report_interval_ < min_interval) report_interval_ = min_interval;
    // Send initial response.
    SendResponse();
  }

  void OnWriteDone(bool ok) override {
    if (!ok) {
      Finish(Status(StatusCode::UNKNOWN, "write failed"));
      return;
    }
    response_.Clear();
    ScheduleTimer();
  }

  void OnCancel() override {
    MaybeCancelTimer();
    Finish(Status(StatusCode::UNKNOWN, "call cancelled by client"));
  }

  void OnDone() override {
    // Free the initial ref from instantiation.
    Unref();
  }

 private:
  void SendResponse() {
    upb::Arena arena;
    xds_data_orca_v3_OrcaLoadReport* response =
        xds_data_orca_v3_OrcaLoadReport_new(arena.ptr());
    {
      grpc::internal::MutexLock lock(&service_->mu_);
      if (service_->cpu_utilization_ != -1) {
        xds_data_orca_v3_OrcaLoadReport_set_cpu_utilization(
            response, service_->cpu_utilization_);
      }
      if (service_->memory_utilization_ != -1) {
        xds_data_orca_v3_OrcaLoadReport_set_mem_utilization(
            response, service_->memory_utilization_);
      }
      for (const auto& p : service_->named_utilization_) {
        xds_data_orca_v3_OrcaLoadReport_utilization_set(
            response,
            upb_StringView_FromDataAndSize(p.first.data(), p.first.size()),
            p.second, arena.ptr());
      }
    }
    size_t buf_length;
    char* buf = xds_data_orca_v3_OrcaLoadReport_serialize(response, arena.ptr(),
                                                          &buf_length);
    Slice response_slice(buf, buf_length);
    ByteBuffer response_buffer(&response_slice, 1);
    response_.Swap(&response_buffer);
    StartWrite(&response_);
  }

  void ScheduleTimer() {
    grpc_core::ApplicationCallbackExecCtx callback_exec_ctx;
    grpc_core::ExecCtx exec_ctx;
    Ref().release();  // Ref held by timer.
    grpc::internal::MutexLock lock(&timer_mu_);
    timer_pending_ = true;
    grpc_timer_init(&timer_, exec_ctx.Now() + report_interval_, &on_timer_);
  }

  void MaybeCancelTimer() {
    grpc_core::ApplicationCallbackExecCtx callback_exec_ctx;
    grpc_core::ExecCtx exec_ctx;
    grpc::internal::MutexLock lock(&timer_mu_);
    if (timer_pending_) {
      timer_pending_ = false;
      grpc_timer_cancel(&timer_);
    }
  }

  static void OnTimer(void* arg, grpc_error_handle error) {
    grpc_core::RefCountedPtr<Reactor> self(static_cast<Reactor*>(arg));
    grpc::internal::MutexLock lock(&self->timer_mu_);
    if (error == GRPC_ERROR_NONE && self->timer_pending_) {
      self->timer_pending_ = false;
      self->SendResponse();
    }
  }

  OrcaService* service_;

  // TODO(roth): Change this to use the EventEngine API once it becomes
  // available.
  grpc::internal::Mutex timer_mu_;
  bool timer_pending_ ABSL_GUARDED_BY(&timer_mu_) = false;
  grpc_timer timer_ ABSL_GUARDED_BY(&timer_mu_);
  grpc_closure on_timer_;

  grpc_core::Duration report_interval_ = grpc_core::Duration::Minutes(1);
  ByteBuffer request_;
  ByteBuffer response_;
};

//
// OrcaService
//

void OrcaService::Register(ServerBuilder* builder) {
  builder->RegisterCallbackGenericService(this);
}

void OrcaService::SetCpuUtilization(double cpu_utilization) {
  grpc::internal::MutexLock lock(&mu_);
  cpu_utilization_ = cpu_utilization;
}

void OrcaService::DeleteCpuUtilization() {
  grpc::internal::MutexLock lock(&mu_);
  cpu_utilization_ = -1;
}

void OrcaService::SetMemoryUtilization(double memory_utilization) {
  grpc::internal::MutexLock lock(&mu_);
  memory_utilization_ = memory_utilization;
}

void OrcaService::DeleteMemoryUtilization() {
  grpc::internal::MutexLock lock(&mu_);
  memory_utilization_ = -1;
}

void OrcaService::SetNamedUtilization(std::string name, double utilization) {
  grpc::internal::MutexLock lock(&mu_);
  named_utilization_[std::move(name)] = utilization;
}

void OrcaService::DeleteNamedUtilization(const std::string& name) {
  grpc::internal::MutexLock lock(&mu_);
  named_utilization_.erase(name);
}

void OrcaService::SetAllNamedUtilization(
    std::map<std::string, double> named_utilization) {
  grpc::internal::MutexLock lock(&mu_);
  named_utilization_ = std::move(named_utilization);
}

ServerGenericBidiReactor* OrcaService::CreateReactor(
    GenericCallbackServerContext* /*ctx*/) {
  return new Reactor(this);
}

}  // namespace experimental
}  // namespace grpc
