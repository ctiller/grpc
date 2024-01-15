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

#ifndef GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_CALL_BATCHER_H
#define GRPC_SRC_CORE_EXT_TRANSPORT_CHAOTIC_GOOD_CALL_BATCHER_H

#include <type_traits>

#include "src/core/ext/transport/chaotic_good/frame.h"
#include "src/core/lib/promise/party.h"
#include "src/core/lib/promise/try_seq.h"
#include "src/core/lib/transport/transport.h"

namespace grpc_core {
namespace chaotic_good {

template <typename Fragment, typename Sender>
class CallBatcher {
 public:
  CallBatcher(Sender sender, uint32_t stream_id)
      : sender_(std::move(sender)), stream_id_(stream_id) {}

  using HeadersType = typename std::remove_reference<
      decltype(std::declval<Fragment>().headers)>::type;

  template <typename Context>
  auto SendHeaders(Context& context, HeadersType headers) {
    return OutgoingFragment(
        context,
        [](const Fragment& fragment) {
          GPR_ASSERT(fragment.headers == nullptr);
          return false;
        },
        [headers = std::move(headers)](Fragment& fragment) mutable {
          fragment.headers = std::move(headers);
          return absl::OkStatus();
        });
  }

  template <typename Context>
  auto SendMessage(Context& context, MessageHandle message,
                   size_t aligned_bytes) {
    return OutgoingFragment(
        context,
        [](const Fragment& fragment) { return fragment.message.has_value(); },
        [message = std::move(message),
         aligned_bytes](Fragment& fragment) mutable {
          const uint32_t message_length = message->payload()->Length();
          const uint32_t padding =
              message_length % aligned_bytes == 0
                  ? 0
                  : aligned_bytes - message_length % aligned_bytes;
          GPR_ASSERT((message_length + padding) % aligned_bytes == 0);
          fragment.message.emplace(std::move(message), padding, message_length);
          return absl::OkStatus();
        });
  }

  template <typename Context>
  auto SendEndOfStream(Context& context) {
    return OutgoingFragment(
        context, [](const Fragment& fragment) { return false; },
        [](Fragment& fragment) {
          fragment.end_of_stream = true;
          return absl::OkStatus();
        });
  }

  template <typename Context>
  auto SendTrailers(Context& context, HeadersType trailers) {
    return OutgoingFragment(
        context,
        [](const Fragment& fragment) {
          GPR_ASSERT(fragment.trailers == nullptr);
          return false;
        },
        [trailers = std::move(trailers)](Fragment& fragment) mutable {
          fragment.trailers = std::move(trailers);
          return absl::OkStatus();
        });
  }

 private:
  auto SendFragment(Fragment fragment) {
    return Map(sender_.Send(std::move(fragment)),
               [](bool success) -> absl::Status {
                 if (!success)
                   return absl::UnavailableError("Transport closed.");
                 return absl::OkStatus();
               });
  }

  template <typename Context, typename FlushFirstFn, typename ConsumeFn>
  auto OutgoingFragment(Context& context, FlushFirstFn check,
                        ConsumeFn consume) {
    absl::optional<Fragment> send_fragment;
    if (fragment_.has_value()) {
      if (check(*fragment_)) {
        send_fragment.emplace(std::move(*fragment_));
        fragment_.emplace();
        fragment_->stream_id = stream_id_;
      }
    } else {
      fragment_.emplace();
      fragment_->stream_id = stream_id_;
      context.SpawnGuarded("flush_batch", [this]() {
        return Seq(static_cast<Party*>(Activity::current())->AfterCurrentPoll(),
                   [this]() {
                     auto fragment = std::move(*fragment_);
                     fragment_.reset();
                     return SendFragment(std::move(fragment));
                   });
      });
    }
    return TrySeq(
        If(
            send_fragment.has_value(),
            [this, &send_fragment]() {
              return SendFragment(std::move(*send_fragment));
            },
            []() -> absl::Status { return absl::OkStatus(); }),
        [this]() -> absl::StatusOr<Fragment*> { return &*fragment_; },
        [consume = std::move(consume)](Fragment* fragment) mutable {
          return consume(*fragment);
        });
  }

  absl::optional<Fragment> fragment_;
  Sender sender_;
  const uint32_t stream_id_;
};

}  // namespace chaotic_good
}  // namespace grpc_core

#endif
