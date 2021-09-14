// Copyright 2021 gRPC authors.
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

#include <grpc/support/port_platform.h>

#include "src/core/lib/config/core_configuration.h"

#include <grpc/support/log.h>

namespace grpc_core {

std::atomic<CoreConfiguration*> CoreConfiguration::config_{nullptr};
std::atomic<CoreConfiguration::RegisteredBuilder*> CoreConfiguration::builders_{
    nullptr};

CoreConfiguration::Builder::Builder() = default;

CoreConfiguration* CoreConfiguration::Builder::Build() {
  return new CoreConfiguration(this);
}

CoreConfiguration::CoreConfiguration(Builder* builder)
    : channel_init_(builder->channel_init_.Build()),
      handshaker_registry_(builder->handshaker_registry_.Build()) {}

void CoreConfiguration::RegisterBuilder(std::function<void(Builder*)> builder) {
  GPR_ASSERT(config_.load(std::memory_order_relaxed) == nullptr);
  RegisteredBuilder* n = new RegisteredBuilder();
  n->builder = std::move(builder);
  n->next = builders_.load(std::memory_order_relaxed);
  while (!builders_.compare_exchange_weak(n->next, n, std::memory_order_acq_rel,
                                          std::memory_order_relaxed)) {
  }
  GPR_ASSERT(config_.load(std::memory_order_relaxed) == nullptr);
}

const CoreConfiguration& CoreConfiguration::BuildNewAndMaybeSet() {
  // Construct builder, pass it up to code that knows about build configuration
  Builder builder;
  BuildCoreConfiguration(&builder);
  for (RegisteredBuilder* b = builders_.load(std::memory_order_acquire);
       b != nullptr; b = b->next) {
    b->builder(&builder);
  }
  // Use builder to construct a confguration
  CoreConfiguration* p = builder.Build();
  // Try to set configuration global - it's possible another thread raced us
  // here, in which case we drop the work we did and use the one that got set
  // first
  CoreConfiguration* expected = nullptr;
  if (!config_.compare_exchange_strong(expected, p, std::memory_order_acq_rel,
                                       std::memory_order_acquire)) {
    delete p;
    return *expected;
  }
  return *p;
}

void CoreConfiguration::Reset() {
  delete config_.exchange(nullptr, std::memory_order_acquire);
  RegisteredBuilder* builder =
      builders_.exchange(nullptr, std::memory_order_acquire);
  while (builder != nullptr) {
    RegisteredBuilder* next = builder->next;
    delete builder;
    builder = next;
  }
}

}  // namespace grpc_core
