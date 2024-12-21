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

#include "test/core/transport/chaotic_good/mock_frame_transport.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/core/lib/promise/loop.h"
#include "src/core/lib/promise/race.h"
#include "src/core/lib/promise/try_seq.h"
#include "test/core/promise/poll_matcher.h"

namespace grpc_core {
namespace chaotic_good {
namespace testing {

MockFrameTransport::~MockFrameTransport() {
  while (!expected_writes_.empty()) {
    auto& w = expected_writes_.front();
    ADD_FAILURE_AT(w.whence.file(), w.whence.line())
        << "Expected write of "
        << absl::ConvertVariantTo<FrameInterface&>(w.frame).ToString();
    expected_writes_.pop();
  }
  if (on_read_done_ != nullptr) {
    on_read_done_(absl::OkStatus());
  }
}

void MockFrameTransport::StartReading(
    Party*, ReadFramePipe::Sender frames,
    absl::AnyInvocable<void(absl::Status)> on_done) {
  reader_ = std::move(frames);
  on_read_done_ = std::move(on_done);
}

void MockFrameTransport::StartWriting(
    Party* party, MpscReceiver<Frame> frames,
    absl::AnyInvocable<void(absl::Status)> on_done) {
  party->Spawn(
      "MockFrameTransport_Writer",
      [this, frames = std::move(frames)]() mutable {
        return Loop([this, frames = std::move(frames)]() mutable {
          return TrySeq(
              frames.Next(), [this](Frame frame) -> LoopCtl<absl::Status> {
                if (closed_.load()) return absl::OkStatus();
                if (expected_writes_.empty()) {
                  ADD_FAILURE()
                      << "Unexpected write of "
                      << absl::ConvertVariantTo<FrameInterface&>(frame)
                             .ToString();
                  return Continue{};
                }
                auto expected = std::move(expected_writes_.front());
                expected_writes_.pop();
                EXPECT_EQ(expected.frame, frame)
                    << " from " << expected.whence.file() << ":"
                    << expected.whence.line();
                return Continue{};
              });
        });
      },
      std::move(on_done));
}

void MockFrameTransport::Read(Frame frame) {
  SliceBuffer buffer;
  auto& frame_interface = absl::ConvertVariantTo<FrameInterface&>(frame);
  LOG(INFO) << "Read " << frame_interface.ToString();
  auto header = frame_interface.MakeHeader();
  frame_interface.SerializePayload(buffer);
  EXPECT_THAT(reader_.Push(IncomingFrame(header, std::move(buffer)))(),
              IsReady());
}

}  // namespace testing
}  // namespace chaotic_good
}  // namespace grpc_core
