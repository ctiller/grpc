// Copyright 2024 The gRPC authors.
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

#ifndef GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_FLOW_CONTROL_H
#define GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_FLOW_CONTROL_H

#include <limits>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "src/core/lib/promise/activity.h"
#include "src/core/lib/promise/poll.h"
#include "src/core/util/sync.h"
#include "src/core/util/useful.h"

namespace grpc_core {

class PromiseBasedFlowController {
 public:
  struct TransportWindowUpdate {
    bool urgent;
    uint32_t window_update;
  };

  struct StreamWindowUpdate {
    bool urgent;
    uint32_t stream_id;
    uint32_t window_update;
  };

  // Instantiate one per transport to buffer stream window sends
  class WindowUpdateAggregator {
   public:
    void AddTransportWindowUpdate(TransportWindowUpdate update) {
      transport_window_update_ += update.window_update;
    }

    void AddStreamWindowUpdate(StreamWindowUpdate update) {
      stream_window_update_[update.stream_id] += update.window_update;
    }

    template <typename F>
    void Flush(F flusher) {
      if (transport_window_update_ != 0) {
        flusher.SendTransportWindowUpdate(
            std::exchange(transport_window_update_, 0));
      }
      while (!stream_window_update_.empty()) {
        auto it = stream_window_update_.begin();
        flusher.SendStreamWindowUpdate(it->first, it->second);
        stream_window_update_.erase(it);
      }
    }

   private:
    using StreamMap = absl::flat_hash_map<uint32_t, uint32_t>;
    uint32_t transport_window_update_ = 0;
    StreamMap stream_window_update_;
  };

  class StreamSend;

  class TransportSend {
   public:
   private:
    friend class StreamSend;
    Mutex mu_;
    uint32_t transport_window_ ABSL_GUARDED_BY(mu_) = 0;
    uint32_t stream_initial_window_ ABSL_GUARDED_BY(mu_) = 0;
    absl::flat_hash_set<StreamSend*> pending_send_requests_
        ABSL_GUARDED_BY(mu_);
  };

  class StreamSend {
   public:
    auto Send(uint32_t bytes_desired) ABSL_LOCKS_EXCLUDED(transport_send_.mu_) {
      MutexLock lock(&transport_send_.mu_);
      return [this, bytes_desired]() -> Poll<uint32_t> {
        MutexLock lock(&transport_send_.mu_);
        auto taken = TakeWindow(bytes_desired);
        if (!taken.has_value()) {
          transport_send_.pending_send_requests_.insert(this);
          send_waker_ = GetContext<Activity>()->MakeNonOwningWaker();
          return Pending{};
        }
        return *taken;
      };
    }

    void ReceiveWindowUpdate(uint32_t bytes) {
      if (bytes == 0) return;
      stream_window_delta_ += bytes;
    }

   private:
    uint32_t stream_window()
        ABSL_EXCLUSIVE_LOCKS_REQUIRED(transport_send_.mu_) {
      return Clamp<int64_t>(
          static_cast<int64_t>(transport_send_.stream_initial_window_) +
              stream_window_delta_,
          0, static_cast<int64_t>(std::numeric_limits<uint32_t>::max()));
    }

    absl::optional<uint32_t> TakeWindow(uint32_t bytes_desired)
        ABSL_EXCLUSIVE_LOCKS_REQUIRED(transport_send_.mu_) {
      if (transport_send_.transport_window_ == 0 || stream_window() == 0) {
        return absl::nullopt;
      }
      const auto take = std::min<uint32_t>(
          {bytes_desired, stream_window(), transport_send_.transport_window_});
      transport_send_.transport_window_ -= take;
      stream_window_delta_ -= take;
      return take;
    }

    TransportSend& transport_send_;
    int64_t stream_window_delta_ = 0;
    Waker send_waker_;
  };

 private:
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_FLOW_CONTROL_H
