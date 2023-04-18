//
//
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
//
//

#include <chrono>
#include <thread>  // NOLINT

#include "absl/strings/escaping.h"
#include "absl/strings/str_cat.h"
#include "gmock/gmock.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"
#include "opencensus/stats/testing/test_utils.h"

#include <grpc++/grpc++.h>
#include <grpcpp/opencensus.h>
#include <grpcpp/support/status.h>

#include "src/core/ext/filters/logging/logging_filter.h"
#include "src/core/lib/config/core_configuration.h"
#include "src/core/lib/gprpp/sync.h"
#include "src/cpp/ext/gcp/observability_logging_sink.h"
#include "src/proto/grpc/testing/echo.grpc.pb.h"
#include "src/proto/grpc/testing/echo_messages.pb.h"
#include "test/core/util/port.h"
#include "test/core/util/test_config.h"
#include "test/cpp/end2end/test_service_impl.h"
#include "test/cpp/ext/filters/census/library.h"
#include "test/cpp/ext/filters/logging/library.h"

namespace grpc {
namespace testing {

using grpc_core::LoggingSink;

using ::testing::AllOf;
using ::testing::Eq;
using ::testing::Field;
using ::testing::Ne;
using ::testing::Pair;
using ::testing::UnorderedElementsAre;

class LoggingCensusIntegrationTest : public LoggingTest {
 protected:
  static void SetUpTestSuite() { LoggingTest::SetUpTestSuite(); }

  static void TearDownTestSuite() { LoggingTest::TearDownTestSuite(); }
};

// Check that exported logs mention information on traces.
TEST_F(LoggingCensusIntegrationTest, Basic) {
  g_test_logging_sink->SetConfig(LoggingSink::Config(4096, 4096));
  std::string expected_trace_id;
  {
    EchoRequest request;
    request.set_message("foo");
    EchoResponse response;
    grpc::ClientContext context;
    ::opencensus::trace::AlwaysSampler always_sampler;
    ::opencensus::trace::StartSpanOptions options;
    options.sampler = &always_sampler;
    auto sampling_span =
        ::opencensus::trace::Span::StartSpan("sampling", nullptr, options);
    grpc::experimental::CensusContext app_census_context(
        "root", &sampling_span, ::opencensus::tags::TagMap{});
    expected_trace_id = app_census_context.Context().trace_id().ToHex();
    context.set_census_context(
        reinterpret_cast<census_context*>(&app_census_context));
    context.AddMetadata("client-key", "client-value");
    traces_recorder_->StartRecording();
    grpc::Status status = stub_->Echo(&context, request, &response);
    EXPECT_TRUE(status.ok());
  }
  absl::SleepFor(absl::Milliseconds(500 * grpc_test_slowdown_factor()));
  ::opencensus::stats::testing::TestUtils::Flush();
  ::opencensus::trace::exporter::SpanExporterTestPeer::ExportForTesting();
  traces_recorder_->StopRecording();
  traces_recorder_->GetAndClearSpans();
  EXPECT_THAT(
      g_test_logging_sink->entries(),
      ::testing::UnorderedElementsAre(
          AllOf(Field(&LoggingSink::Entry::type,
                      Eq(LoggingSink::Entry::EventType::kClientHeader)),
                Field(&LoggingSink::Entry::logger,
                      Eq(LoggingSink::Entry::Logger::kClient)),
                Field(&LoggingSink::Entry::authority, Eq(server_address_)),
                Field(&LoggingSink::Entry::service_name,
                      Eq("grpc.testing.EchoTestService")),
                Field(&LoggingSink::Entry::method_name, Eq("Echo")),
                Field(&LoggingSink::Entry::payload,
                      Field(&LoggingSink::Entry::Payload::metadata,
                            UnorderedElementsAre(
                                Pair("client-key", "client-value")))),
                Field(&LoggingSink::Entry::trace_id, Eq(expected_trace_id)),
                Field(&LoggingSink::Entry::span_id, Ne("")),
                Field(&LoggingSink::Entry::is_sampled, true)),
          AllOf(Field(&LoggingSink::Entry::type,
                      Eq(LoggingSink::Entry::EventType::kClientMessage)),
                Field(&LoggingSink::Entry::logger,
                      Eq(LoggingSink::Entry::Logger::kClient)),
                Field(&LoggingSink::Entry::authority, Eq(server_address_)),
                Field(&LoggingSink::Entry::service_name,
                      Eq("grpc.testing.EchoTestService")),
                Field(&LoggingSink::Entry::method_name, Eq("Echo")),
                Field(&LoggingSink::Entry::payload,
                      AllOf(Field(&LoggingSink::Entry::Payload::message_length,
                                  Eq(5)),
                            Field(&LoggingSink::Entry::Payload::message,
                                  Eq("\n\003foo")))),
                Field(&LoggingSink::Entry::trace_id, Eq(expected_trace_id)),
                Field(&LoggingSink::Entry::span_id, Ne("")),
                Field(&LoggingSink::Entry::is_sampled, true)),
          AllOf(Field(&LoggingSink::Entry::type,
                      Eq(LoggingSink::Entry::EventType::kClientHalfClose)),
                Field(&LoggingSink::Entry::logger,
                      Eq(LoggingSink::Entry::Logger::kClient)),
                Field(&LoggingSink::Entry::authority, Eq(server_address_)),
                Field(&LoggingSink::Entry::service_name,
                      Eq("grpc.testing.EchoTestService")),
                Field(&LoggingSink::Entry::method_name, Eq("Echo")),
                Field(&LoggingSink::Entry::trace_id, Eq(expected_trace_id)),
                Field(&LoggingSink::Entry::span_id, Ne("")),
                Field(&LoggingSink::Entry::is_sampled, true)),
          AllOf(Field(&LoggingSink::Entry::type,
                      Eq(LoggingSink::Entry::EventType::kServerHeader)),
                Field(&LoggingSink::Entry::logger,
                      Eq(LoggingSink::Entry::Logger::kClient)),
                Field(&LoggingSink::Entry::authority, Eq(server_address_)),
                Field(&LoggingSink::Entry::service_name,
                      Eq("grpc.testing.EchoTestService")),
                Field(&LoggingSink::Entry::method_name, Eq("Echo")),
                Field(&LoggingSink::Entry::payload,
                      Field(&LoggingSink::Entry::Payload::metadata,
                            UnorderedElementsAre(Pair("server-header-key",
                                                      "server-header-value")))),
                Field(&LoggingSink::Entry::trace_id, Eq(expected_trace_id)),
                Field(&LoggingSink::Entry::span_id, Ne("")),
                Field(&LoggingSink::Entry::is_sampled, true)),
          AllOf(Field(&LoggingSink::Entry::type,
                      Eq(LoggingSink::Entry::EventType::kServerMessage)),
                Field(&LoggingSink::Entry::logger,
                      Eq(LoggingSink::Entry::Logger::kClient)),
                Field(&LoggingSink::Entry::authority, Eq(server_address_)),
                Field(&LoggingSink::Entry::service_name,
                      Eq("grpc.testing.EchoTestService")),
                Field(&LoggingSink::Entry::method_name, Eq("Echo")),
                Field(&LoggingSink::Entry::payload,
                      AllOf(Field(&LoggingSink::Entry::Payload::message_length,
                                  Eq(5)),
                            Field(&LoggingSink::Entry::Payload::message,
                                  Eq("\n\003foo")))),
                Field(&LoggingSink::Entry::trace_id, Eq(expected_trace_id)),
                Field(&LoggingSink::Entry::span_id, Ne("")),
                Field(&LoggingSink::Entry::is_sampled, true)),
          AllOf(
              Field(&LoggingSink::Entry::type,
                    Eq(LoggingSink::Entry::EventType::kServerTrailer)),
              Field(&LoggingSink::Entry::logger,
                    Eq(LoggingSink::Entry::Logger::kClient)),
              Field(&LoggingSink::Entry::authority, Eq(server_address_)),
              Field(&LoggingSink::Entry::service_name,
                    Eq("grpc.testing.EchoTestService")),
              Field(&LoggingSink::Entry::method_name, Eq("Echo")),
              Field(&LoggingSink::Entry::payload,
                    Field(&LoggingSink::Entry::Payload::metadata,
                          UnorderedElementsAre(Pair("server-trailer-key",
                                                    "server-trailer-value")))),
              Field(&LoggingSink::Entry::trace_id, Eq(expected_trace_id)),
              Field(&LoggingSink::Entry::span_id, Ne("")),
              Field(&LoggingSink::Entry::is_sampled, true)),
          AllOf(Field(&LoggingSink::Entry::type,
                      Eq(LoggingSink::Entry::EventType::kClientHeader)),
                Field(&LoggingSink::Entry::logger,
                      Eq(LoggingSink::Entry::Logger::kServer)),
                Field(&LoggingSink::Entry::authority, Eq(server_address_)),
                Field(&LoggingSink::Entry::service_name,
                      Eq("grpc.testing.EchoTestService")),
                Field(&LoggingSink::Entry::method_name, Eq("Echo")),
                Field(&LoggingSink::Entry::payload,
                      Field(&LoggingSink::Entry::Payload::metadata,
                            UnorderedElementsAre(
                                Pair("client-key", "client-value")))),
                Field(&LoggingSink::Entry::trace_id, Eq(expected_trace_id)),
                Field(&LoggingSink::Entry::span_id, Ne("")),
                Field(&LoggingSink::Entry::is_sampled, true)),
          AllOf(Field(&LoggingSink::Entry::type,
                      Eq(LoggingSink::Entry::EventType::kClientMessage)),
                Field(&LoggingSink::Entry::logger,
                      Eq(LoggingSink::Entry::Logger::kServer)),
                Field(&LoggingSink::Entry::authority, Eq(server_address_)),
                Field(&LoggingSink::Entry::service_name,
                      Eq("grpc.testing.EchoTestService")),
                Field(&LoggingSink::Entry::method_name, Eq("Echo")),
                Field(&LoggingSink::Entry::payload,
                      AllOf(Field(&LoggingSink::Entry::Payload::message_length,
                                  Eq(5)),
                            Field(&LoggingSink::Entry::Payload::message,
                                  Eq("\n\003foo")))),
                Field(&LoggingSink::Entry::trace_id, Eq(expected_trace_id)),
                Field(&LoggingSink::Entry::span_id, Ne("")),
                Field(&LoggingSink::Entry::is_sampled, true)),
          AllOf(Field(&LoggingSink::Entry::type,
                      Eq(LoggingSink::Entry::EventType::kClientHalfClose)),
                Field(&LoggingSink::Entry::logger,
                      Eq(LoggingSink::Entry::Logger::kServer)),
                Field(&LoggingSink::Entry::authority, Eq(server_address_)),
                Field(&LoggingSink::Entry::service_name,
                      Eq("grpc.testing.EchoTestService")),
                Field(&LoggingSink::Entry::method_name, Eq("Echo")),
                Field(&LoggingSink::Entry::trace_id, Eq(expected_trace_id)),
                Field(&LoggingSink::Entry::span_id, Ne("")),
                Field(&LoggingSink::Entry::is_sampled, true)),
          AllOf(Field(&LoggingSink::Entry::type,
                      Eq(LoggingSink::Entry::EventType::kServerHeader)),
                Field(&LoggingSink::Entry::logger,
                      Eq(LoggingSink::Entry::Logger::kServer)),
                Field(&LoggingSink::Entry::authority, Eq(server_address_)),
                Field(&LoggingSink::Entry::service_name,
                      Eq("grpc.testing.EchoTestService")),
                Field(&LoggingSink::Entry::method_name, Eq("Echo")),
                Field(&LoggingSink::Entry::payload,
                      Field(&LoggingSink::Entry::Payload::metadata,
                            UnorderedElementsAre(Pair("server-header-key",
                                                      "server-header-value")))),
                Field(&LoggingSink::Entry::trace_id, Eq(expected_trace_id)),
                Field(&LoggingSink::Entry::span_id, Ne("")),
                Field(&LoggingSink::Entry::is_sampled, true)),
          AllOf(Field(&LoggingSink::Entry::type,
                      Eq(LoggingSink::Entry::EventType::kServerMessage)),
                Field(&LoggingSink::Entry::logger,
                      Eq(LoggingSink::Entry::Logger::kServer)),
                Field(&LoggingSink::Entry::authority, Eq(server_address_)),
                Field(&LoggingSink::Entry::service_name,
                      Eq("grpc.testing.EchoTestService")),
                Field(&LoggingSink::Entry::method_name, Eq("Echo")),
                Field(&LoggingSink::Entry::payload,
                      AllOf(Field(&LoggingSink::Entry::Payload::message_length,
                                  Eq(5)),
                            Field(&LoggingSink::Entry::Payload::message,
                                  Eq("\n\003foo")))),
                Field(&LoggingSink::Entry::trace_id, Eq(expected_trace_id)),
                Field(&LoggingSink::Entry::span_id, Ne("")),
                Field(&LoggingSink::Entry::is_sampled, true)),
          AllOf(
              Field(&LoggingSink::Entry::type,
                    Eq(LoggingSink::Entry::EventType::kServerTrailer)),
              Field(&LoggingSink::Entry::logger,
                    Eq(LoggingSink::Entry::Logger::kServer)),
              Field(&LoggingSink::Entry::authority, Eq(server_address_)),
              Field(&LoggingSink::Entry::service_name,
                    Eq("grpc.testing.EchoTestService")),
              Field(&LoggingSink::Entry::method_name, Eq("Echo")),
              Field(&LoggingSink::Entry::payload,
                    Field(&LoggingSink::Entry::Payload::metadata,
                          UnorderedElementsAre(Pair("server-trailer-key",
                                                    "server-trailer-value")))),
              Field(&LoggingSink::Entry::trace_id, Eq(expected_trace_id)),
              Field(&LoggingSink::Entry::span_id, Ne("")),
              Field(&LoggingSink::Entry::is_sampled, true))));
}

}  // namespace testing
}  // namespace grpc

int main(int argc, char** argv) {
  grpc::testing::TestEnvironment env(&argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
