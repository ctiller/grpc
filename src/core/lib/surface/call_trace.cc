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

#include "src/core/lib/surface/call_trace.h"

#include "absl/container/flat_hash_map.h"

#include <grpc/support/log.h>

#include "src/core/lib/channel/channel_stack.h"

namespace grpc_core {

const grpc_channel_filter* PromiseTracingFilterFor(
    const grpc_channel_filter* filter) {
  struct DerivedFilter : public grpc_channel_filter {
    explicit DerivedFilter(const grpc_channel_filter* filter)
        : grpc_channel_filter{
              /* start_transport_stream_op_batch: */ grpc_call_next_op,
              /* make_call_promise: */
              [](grpc_channel_element* elem, grpc_core::CallArgs call_args,
                 grpc_core::NextPromiseFactory next_promise_factory)
                  -> grpc_core::ArenaPromise<grpc_core::ServerMetadataHandle> {
                auto* filter =
                    static_cast<const DerivedFilter*>(elem->filter)->filter;
                gpr_log(
                    GPR_DEBUG,
                    "%sCreateCallPromise[%s]: client_initial_metadata=%s",
                    Activity::current()->DebugTag().c_str(), filter->name,
                    call_args.client_initial_metadata->DebugString().c_str());
                return [filter, child = next_promise_factory(
                                    std::move(call_args))]() mutable {
                  gpr_log(GPR_DEBUG, "%sPollCallPromise[%s]: begin",
                          Activity::current()->DebugTag().c_str(),
                          filter->name);
                  auto r = child();
                  if (auto* p = absl::get_if<ServerMetadataHandle>(&r)) {
                    gpr_log(GPR_DEBUG, "%sPollCallPromise[%s]: done: %s",
                            Activity::current()->DebugTag().c_str(),
                            filter->name, (*p)->DebugString().c_str());
                  } else {
                    gpr_log(GPR_DEBUG, "%sPollCallPromise[%s]: <<pending>",
                            Activity::current()->DebugTag().c_str(),
                            filter->name);
                  }
                  return r;
                };
              },
              grpc_channel_next_op, /* sizeof_call_data: */ 0,
              /* init_call_elem: */
              [](grpc_call_element* elem, const grpc_call_element_args* args) {
                return absl::OkStatus();
              },
              grpc_call_stack_ignore_set_pollset_or_pollset_set,
              /* destroy_call_elem: */
              [](grpc_call_element* elem,
                 const grpc_call_final_info* final_info,
                 grpc_closure* then_schedule_closure) {},
              /* sizeof_channel_data: */ 0, /* init_channel_elem: */
              [](grpc_channel_element* elem, grpc_channel_element_args* args) {
                return absl::OkStatus();
              },
              /* post_init_channel_elem: */
              [](grpc_channel_stack* stk, grpc_channel_element* elem) {},
              /* destroy_channel_elem: */ [](grpc_channel_element* elem) {},
              grpc_channel_next_get_info, filter->name},
          filter(filter) {}
    const grpc_channel_filter* const filter;
  };
  struct Globals {
    Mutex mu;
    absl::flat_hash_map<const grpc_channel_filter*,
                        std::unique_ptr<DerivedFilter>>
        map;
  };
  auto* globals = NoDestructSingleton<Globals>::Get();
  MutexLock lock(&globals->mu);
  auto it = globals->map.find(filter);
  if (it != globals->map.end()) return it->second.get();
  return globals->map.emplace(filter, absl::make_unique<DerivedFilter>(filter))
      .first->second.get();
}

}  // namespace grpc_core
