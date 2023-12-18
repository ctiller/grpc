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

#ifndef GRPC_SRC_CORE_LIB_PROMISE_MERGE_BUFFER_H
#define GRPC_SRC_CORE_LIB_PROMISE_MERGE_BUFFER_H

namespace grpc_core {

template <typename T, class Flusher>
class MergeBuffer {
 public:
  explicit MergeBuffer(Flusher flusher) : flusher_(std::move(flusher)) {}

  template <typename MergeFn>
  auto Merge(MergeFn merge_fn) {
    auto add_value = merge_fn(buffer_.has_value() ? &*buffer_ : nullptr);
    return If(
        buffer_.has_value() && add_value.has_value(),
        [this, &add_value]() mutable {
          return flusher_(std::exchange(*buffer_, *add_value));
        },
        [this, &add_value]() {
          if (add_value.has_value()) {
            buffer_ = std::move(*add_value);
            static_cast<Party*>(Activity::current())
                ->SpawnFinally("merge-buffer-flush", );
          }
          return Immediate(StatusFlag(true));
        });
  }

 private:
  absl::optional<T> buffer_;
  Flusher flusher_;
};

}  // namespace grpc_core

#endif
