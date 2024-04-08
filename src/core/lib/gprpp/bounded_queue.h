// Copyright 2023 gRPC authors.
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

#ifndef GRPC_SRC_CORE_LIB_GPRPP_BOUNDED_QUEUE_H
#define GRPC_SRC_CORE_LIB_GPRPP_BOUNDED_QUEUE_H

#include <grpc/support/port_platform.h>

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace grpc_core {

template <typename T, uint16_t kMaxElems>
class BoundedQueue {
 public:
  // Push value into the queue, returns true if successful (and moves-from
  // value), false if not (in which case value is untouched).
  bool Push(T& value) {
    size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
    while (true) {
      auto* cell = &cells_[pos % kMaxElems];
      size_t seq = cell->seq.load(std::memory_order_acquire);
      intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);
      if (diff == 0) {
        if (enqueue_pos_.compare_exchange_weak(pos, pos + 1,
                                               std::memory_order_relaxed)) {
          cell->value = std::move(value);
          cell->seq.store(pos + 1, std::memory_order_release);
          return true;
        }
      } else if (diff < 0) {
        return false;
      } else {
        pos = enqueue_pos_.load(std::memory_order_relaxed);
      }
    }
  }

  // Remove a value from the queue. Returns nullopt if the queue was empty.

 private:
  struct Cell {
    union {
      T value;
    };
    std::atomic<size_t> seq;
  };
  alignas(GPR_CACHELINE_SIZE) std::atomic<size_t> enqueue_pos_;
  alignas(GPR_CACHELINE_SIZE) std::atomic<size_t> dequeue_pos_;
  alignas(GPR_CACHELINE_SIZE) Cell cells_[kMaxElems];
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_GPRPP_BOUNDED_QUEUE_H
