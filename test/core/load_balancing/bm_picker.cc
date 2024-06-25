// Copyright 2024 gRPC authors.
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

#include <memory>

#include <benchmark/benchmark.h>

#include "absl/strings/string_view.h"

#include <grpc/grpc.h>

#include "src/core/client_channel/subchannel_interface_internal.h"
#include "src/core/lib/address_utils/parse_address.h"
#include "src/core/lib/config/core_configuration.h"
#include "src/core/lib/event_engine/default_event_engine.h"
#include "src/core/lib/transport/connectivity_state.h"
#include "src/core/load_balancing/health_check_client_internal.h"
#include "src/core/load_balancing/lb_policy.h"
#include "src/core/util/json/json_reader.h"

namespace grpc_core {
namespace {

class BenchmarkHelper : public std::enable_shared_from_this<BenchmarkHelper> {
 public:
  BenchmarkHelper(absl::string_view name, absl::string_view config)
      : name_(name), config_json_(config) {
    CHECK(lb_policy_ != nullptr) << "Failed to create LB policy: " << name;
    auto parsed_json = JsonParse(std::string(config_json_));
    CHECK_OK(parsed_json);
    auto config_parsed =
        CoreConfiguration::Get().lb_policy_registry().ParseLoadBalancingConfig(
            *parsed_json);
    CHECK_OK(config_parsed);
    config_ = std::move(*config_parsed);
  }

  RefCountedPtr<LoadBalancingPolicy::SubchannelPicker> GetPicker() {
    MutexLock lock(&mu_);
    while (picker_ == nullptr) {
      cv_.Wait(&mu_);
    }
    return picker_;
  }

  void UpdateState(size_t num_endpoints) {
    {
      MutexLock lock(&mu_);
      picker_ = nullptr;
      work_serializer_->Schedule(
          [this, num_endpoints]() {
            EndpointAddressesList addresses;
            for (size_t i = 0; i < num_endpoints; i++) {
              grpc_resolved_address addr;
              int port = i % 65536;
              int ip = i / 65536;
              CHECK(ip < 256);
              CHECK(grpc_parse_uri(
                  URI::Parse(absl::StrCat("ipv4:127.0.0.", ip, ":", port))
                      .value(),
                  &addr));
              addresses.emplace_back(addr, ChannelArgs());
            }
            CHECK_OK(lb_policy_->UpdateLocked(LoadBalancingPolicy::UpdateArgs{
                std::make_shared<EndpointAddressesListIterator>(
                    std::move(addresses)),
                config_, "", ChannelArgs()}));
          },
          DEBUG_LOCATION);
    }
    work_serializer_->DrainQueue();
  }

 private:
  class SubchannelFake final : public SubchannelInterface {
   public:
    explicit SubchannelFake(BenchmarkHelper* helper) : helper_(helper) {}

    void WatchConnectivityState(
        std::unique_ptr<ConnectivityStateWatcherInterface> unique_watcher)
        override {
      AddConnectivityWatcherInternal(
          std::shared_ptr<ConnectivityStateWatcherInterface>(
              std::move(unique_watcher)));
    }

    void CancelConnectivityStateWatch(
        ConnectivityStateWatcherInterface* watcher) override {
      MutexLock lock(&helper_->mu_);
      helper_->connectivity_watchers_.erase(watcher);
    }

    void RequestConnection() override { LOG(FATAL) << "unimplemented"; }

    void ResetBackoff() override { LOG(FATAL) << "unimplemented"; }

    void AddDataWatcher(
        std::unique_ptr<DataWatcherInterface> watcher) override {
      auto* watcher_internal =
          DownCast<InternalSubchannelDataWatcherInterface*>(watcher.get());
      if (watcher_internal->type() == HealthProducer::Type()) {
        AddConnectivityWatcherInternal(
            DownCast<HealthWatcher*>(watcher_internal)->TakeWatcher());
      } else {
        LOG(FATAL) << "unimplemented watcher type: "
                   << watcher_internal->type();
      }
    }

    void CancelDataWatcher(DataWatcherInterface* watcher) override {
      // LOG(FATAL) << "unimplemented";
    }

   private:
    void AddConnectivityWatcherInternal(
        std::shared_ptr<ConnectivityStateWatcherInterface> watcher) {
      {
        MutexLock lock(&helper_->mu_);
        helper_->work_serializer_->Schedule(
            [watcher]() {
              watcher->OnConnectivityStateChange(GRPC_CHANNEL_READY,
                                                 absl::OkStatus());
            },
            DEBUG_LOCATION);
        helper_->connectivity_watchers_.insert(std::move(watcher));
      }
      helper_->work_serializer_->DrainQueue();
    }

    BenchmarkHelper* helper_;
  };

  class ChannelControl final
      : public LoadBalancingPolicy::ChannelControlHelper {
   public:
    explicit ChannelControl(BenchmarkHelper* helper) : helper_(helper) {}

    RefCountedPtr<SubchannelInterface> CreateSubchannel(
        const grpc_resolved_address& address,
        const ChannelArgs& per_address_args, const ChannelArgs& args) override {
      return MakeRefCounted<SubchannelFake>(helper_);
    }

    void UpdateState(
        grpc_connectivity_state state, const absl::Status& status,
        RefCountedPtr<LoadBalancingPolicy::SubchannelPicker> picker) override {
      MutexLock lock(&helper_->mu_);
      helper_->picker_ = std::move(picker);
      helper_->cv_.SignalAll();
    }

    void RequestReresolution() override { LOG(FATAL) << "unimplemented"; }

    absl::string_view GetTarget() override { return "foo"; }

    absl::string_view GetAuthority() override { return "foo"; }

    RefCountedPtr<grpc_channel_credentials> GetChannelCredentials() override {
      LOG(FATAL) << "unimplemented";
    }

    RefCountedPtr<grpc_channel_credentials> GetUnsafeChannelCredentials()
        override {
      LOG(FATAL) << "unimplemented";
    }

    grpc_event_engine::experimental::EventEngine* GetEventEngine() override {
      return helper_->event_engine_.get();
    }

    GlobalStatsPluginRegistry::StatsPluginGroup& GetStatsPluginGroup()
        override {
      return helper_->stats_plugin_group_;
    }

    void AddTraceEvent(TraceSeverity severity,
                       absl::string_view message) override {
      LOG(FATAL) << "unimplemented";
    }

    BenchmarkHelper* helper_;
  };

  const absl::string_view name_;
  const absl::string_view config_json_;
  std::shared_ptr<grpc_event_engine::experimental::EventEngine> event_engine_ =
      grpc_event_engine::experimental::GetDefaultEventEngine();
  std::shared_ptr<WorkSerializer> work_serializer_ =
      std::make_shared<WorkSerializer>(event_engine_);
  OrphanablePtr<LoadBalancingPolicy> lb_policy_ =
      CoreConfiguration::Get().lb_policy_registry().CreateLoadBalancingPolicy(
          name_, LoadBalancingPolicy::Args{
                     work_serializer_, std::make_unique<ChannelControl>(this),
                     ChannelArgs()});
  RefCountedPtr<LoadBalancingPolicy::Config> config_;
  Mutex mu_;
  CondVar cv_;
  RefCountedPtr<LoadBalancingPolicy::SubchannelPicker> picker_
      ABSL_GUARDED_BY(mu_);
  absl::flat_hash_set<
      std::shared_ptr<SubchannelInterface::ConnectivityStateWatcherInterface>>
      connectivity_watchers_ ABSL_GUARDED_BY(mu_);
  GlobalStatsPluginRegistry::StatsPluginGroup stats_plugin_group_ =
      GlobalStatsPluginRegistry::GetStatsPluginsForChannel(
          experimental::StatsPluginChannelScope("foo", "foo"));
};

#define BACKEND_RANGE RangeMultiplier(10)->Range(1, 100000)

#define BENCHMARK_HELPER(name, config)                       \
  ([]() -> BenchmarkHelper& {                                \
    static auto* helper = new BenchmarkHelper(name, config); \
    return *helper;                                          \
  }())

#define PICKER_BENCHMARK(name)                                               \
  BENCHMARK_CAPTURE(name, pick_first,                                        \
                    BENCHMARK_HELPER("pick_first", "[{\"pick_first\":{}}]")) \
      ->BACKEND_RANGE;                                                       \
  BENCHMARK_CAPTURE(                                                         \
      name, weighted_round_robin,                                            \
      BENCHMARK_HELPER(                                                      \
          "weighted_round_robin",                                            \
          "[{\"weighted_round_robin\":{\"enableOobLoadReport\":false}}]"))   \
      ->BACKEND_RANGE;

void BM_Pick(benchmark::State& state, BenchmarkHelper& helper) {
  helper.UpdateState(state.range(0));
  auto picker = helper.GetPicker();
  for (auto _ : state) {
    picker->Pick(LoadBalancingPolicy::PickArgs{
        "/foo/bar",
        nullptr,
        nullptr,
    });
  }
}
PICKER_BENCHMARK(BM_Pick);

}  // namespace
}  // namespace grpc_core

// Some distros have RunSpecifiedBenchmarks under the benchmark namespace,
// and others do not. This allows us to support both modes.
namespace benchmark {
void RunTheBenchmarksNamespaced() { RunSpecifiedBenchmarks(); }
}  // namespace benchmark

int main(int argc, char** argv) {
  ::benchmark::Initialize(&argc, argv);
  grpc_init();
  benchmark::RunTheBenchmarksNamespaced();
  grpc_shutdown();
  return 0;
}
