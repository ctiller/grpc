//
//
// Copyright 2016 gRPC authors.
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

#include <grpc/support/port_platform.h>

#include "src/core/lib/surface/channel_init.h"

#include <string.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <type_traits>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"

#include <grpc/support/log.h>

#include "src/core/lib/channel/channel_stack_trace.h"
#include "src/core/lib/debug/trace.h"
#include "src/core/lib/gprpp/crash.h"
#include "src/core/lib/surface/channel_stack_type.h"

namespace grpc_core {

const char* (*NameFromChannelFilter)(const grpc_channel_filter*);

namespace {
struct CompareChannelFiltersByName {
  bool operator()(const grpc_channel_filter* a,
                  const grpc_channel_filter* b) const {
    return strcmp(NameFromChannelFilter(a), NameFromChannelFilter(b)) < 0;
  }
};
}  // namespace

ChannelInit::FilterRegistration& ChannelInit::FilterRegistration::After(
    std::initializer_list<const grpc_channel_filter*> filters) {
  for (auto filter : filters) {
    after_.push_back(filter);
  }
  return *this;
}

ChannelInit::FilterRegistration& ChannelInit::FilterRegistration::Before(
    std::initializer_list<const grpc_channel_filter*> filters) {
  for (auto filter : filters) {
    before_.push_back(filter);
  }
  return *this;
}

ChannelInit::FilterRegistration& ChannelInit::FilterRegistration::If(
    InclusionPredicate predicate) {
  predicates_.emplace_back(std::move(predicate));
  return *this;
}

ChannelInit::FilterRegistration& ChannelInit::Builder::RegisterFilter(
    grpc_channel_stack_type type, const grpc_channel_filter* filter,
    SourceLocation registration_source) {
  filters_[type].emplace_back(
      std::make_unique<FilterRegistration>(filter, registration_source));
  return *filters_[type].back();
}

ChannelInit::StackConfig ChannelInit::BuildStackConfig(
    const std::vector<std::unique_ptr<ChannelInit::FilterRegistration>>&
        registrations,
    PostProcessor* post_processors, grpc_channel_stack_type type) {
  auto collapse_predicates =
      [](std::vector<InclusionPredicate> predicates) -> InclusionPredicate {
    switch (predicates.size()) {
      case 0:
        return [](const ChannelArgs&) { return true; };
      case 1:
        return std::move(predicates[0]);
      default:
        return [predicates = std::move(predicates)](const ChannelArgs& args) {
          for (auto& predicate : predicates) {
            if (!predicate(args)) return false;
          }
          return true;
        };
    }
  };
  // Phase 1: Build a map from filter to the set of filters that must be
  // initialized before it.
  // We order this map (and the set of dependent filters) by filter name to
  // ensure algorithm ordering stability is deterministic for a given build.
  // We should not require this, but at the time of writing it's expected that
  // this will help overall stability.
  using F = const grpc_channel_filter*;
  std::map<F, FilterRegistration*> filter_to_registration;
  using DependencyMap = std::map<F, std::set<F, CompareChannelFiltersByName>,
                                 CompareChannelFiltersByName>;
  DependencyMap dependencies;
  std::vector<Filter> terminal_filters;
  for (const auto& registration : registrations) {
    if (filter_to_registration.count(registration->filter_) > 0) {
      const auto first =
          filter_to_registration[registration->filter_]->registration_source_;
      const auto second = registration->registration_source_;
      Crash(absl::StrCat("Duplicate registration of channel filter ",
                         NameFromChannelFilter(registration->filter_),
                         "\nfirst: ", first.file(), ":", first.line(),
                         "\nsecond: ", second.file(), ":", second.line()));
    }
    filter_to_registration[registration->filter_] = registration.get();
    if (registration->terminal_) {
      GPR_ASSERT(registration->after_.empty());
      GPR_ASSERT(registration->before_.empty());
      GPR_ASSERT(!registration->before_all_);
      terminal_filters.emplace_back(
          registration->filter_,
          collapse_predicates(std::move(registration->predicates_)));
    } else {
      dependencies[registration->filter_];  // Ensure it's in the map.
    }
  }
  for (const auto& registration : registrations) {
    if (registration->terminal_) continue;
    GPR_ASSERT(filter_to_registration.count(registration->filter_) > 0);
    for (F after : registration->after_) {
      if (filter_to_registration.count(after) == 0) {
        Crash(absl::StrCat(
            "Filter ", NameFromChannelFilter(after),
            " not registered, but is referenced in the after clause of ",
            NameFromChannelFilter(registration->filter_),
            " when building channel stack ",
            grpc_channel_stack_type_string(type)));
      }
      dependencies[registration->filter_].insert(after);
    }
    for (F before : registration->before_) {
      if (filter_to_registration.count(before) == 0) {
        Crash(absl::StrCat(
            "Filter ", NameFromChannelFilter(before),
            " not registered, but is referenced in the after clause of ",
            NameFromChannelFilter(registration->filter_),
            " when building channel stack ",
            grpc_channel_stack_type_string(type)));
      }
      dependencies[before].insert(registration->filter_);
    }
    if (registration->before_all_) {
      for (const auto& other : registrations) {
        if (other.get() == registration.get()) continue;
        if (other->terminal_) continue;
        dependencies[other->filter_].insert(registration->filter_);
      }
    }
  }
  // Phase 2: Build a list of filters in dependency order.
  // We can simply iterate through and add anything with no dependency.
  // We then remove that filter from the dependency list of all other filters.
  // We repeat until we have no more filters to add.
  auto build_remaining_dependency_graph =
      [](const DependencyMap& dependencies) {
        std::string result;
        for (const auto& p : dependencies) {
          absl::StrAppend(&result, NameFromChannelFilter(p.first), " ->");
          for (const auto& d : p.second) {
            absl::StrAppend(&result, " ", NameFromChannelFilter(d));
          }
          absl::StrAppend(&result, "\n");
        }
        return result;
      };
  const DependencyMap original = dependencies;
  auto take_ready_dependency = [&]() {
    for (auto it = dependencies.begin(); it != dependencies.end(); ++it) {
      if (it->second.empty()) {
        auto r = it->first;
        dependencies.erase(it);
        return r;
      }
    }
    Crash(absl::StrCat(
        "Unresolvable graph of channel filters - remaining graph:\n",
        build_remaining_dependency_graph(dependencies), "original:\n",
        build_remaining_dependency_graph(original)));
  };
  std::vector<Filter> filters;
  while (!dependencies.empty()) {
    auto filter = take_ready_dependency();
    filters.emplace_back(
        filter, collapse_predicates(
                    std::move(filter_to_registration[filter]->predicates_)));
    for (auto& p : dependencies) {
      p.second.erase(filter);
    }
  }
  std::vector<PostProcessor> post_processor_functions;
  for (int i = 0; i < static_cast<int>(PostProcessorSlot::kCount); i++) {
    if (post_processors[i] == nullptr) continue;
    post_processor_functions.emplace_back(std::move(post_processors[i]));
  }
  if (grpc_trace_channel_stack.enabled()) {
    gpr_log(GPR_INFO,
            "ORDERED CHANNEL STACK %s:", grpc_channel_stack_type_string(type));
    std::map<const grpc_channel_filter*, std::string> loc_strs;
    size_t max_loc_str_len = 0;
    size_t max_filter_name_len = 0;
    auto add_loc_str = [&max_loc_str_len, &loc_strs, &filter_to_registration,
                        &max_filter_name_len](
                           const grpc_channel_filter* filter) {
      max_filter_name_len =
          std::max(strlen(NameFromChannelFilter(filter)), max_filter_name_len);
      const auto registration =
          filter_to_registration[filter]->registration_source_;
      absl::string_view file = registration.file();
      auto slash_pos = file.rfind('/');
      if (slash_pos != file.npos) {
        file = file.substr(slash_pos + 1);
      }
      auto loc_str = absl::StrCat(file, ":", registration.line(), ":");
      max_loc_str_len = std::max(max_loc_str_len, loc_str.length());
      loc_strs.emplace(filter, std::move(loc_str));
    };
    for (const auto& filter : filters) {
      add_loc_str(filter.filter);
    }
    for (const auto& terminal : terminal_filters) {
      add_loc_str(terminal.filter);
    }
    for (auto& loc_str : loc_strs) {
      loc_str.second = absl::StrCat(
          loc_str.second,
          std::string(max_loc_str_len + 2 - loc_str.second.length(), ' '));
    }
    for (const auto& filter : filters) {
      auto dep_it = original.find(filter.filter);
      std::string after_str;
      if (dep_it != original.end() && !dep_it->second.empty()) {
        after_str = absl::StrCat(
            std::string(max_filter_name_len + 1 -
                            strlen(NameFromChannelFilter(filter.filter)),
                        ' '),
            "after ",
            absl::StrJoin(
                dep_it->second, ", ",
                [](std::string* out, const grpc_channel_filter* filter) {
                  out->append(NameFromChannelFilter(filter));
                }));
      }
      const auto filter_str =
          absl::StrCat("  ", loc_strs[filter.filter],
                       NameFromChannelFilter(filter.filter), after_str);
      gpr_log(GPR_INFO, "%s", filter_str.c_str());
    }
    for (const auto& terminal : terminal_filters) {
      const auto filter_str = absl::StrCat(
          "  ", loc_strs[terminal.filter],
          NameFromChannelFilter(terminal.filter),
          std::string(max_filter_name_len + 1 -
                          strlen(NameFromChannelFilter(terminal.filter)),
                      ' '),
          "[terminal]");
      gpr_log(GPR_INFO, "%s", filter_str.c_str());
    }
  }
  return StackConfig{std::move(filters), std::move(terminal_filters),
                     std::move(post_processor_functions)};
};

ChannelInit ChannelInit::Builder::Build() {
  ChannelInit result;
  for (int i = 0; i < GRPC_NUM_CHANNEL_STACK_TYPES; i++) {
    result.stack_configs_[i] =
        BuildStackConfig(filters_[i], post_processors_[i],
                         static_cast<grpc_channel_stack_type>(i));
  }
  return result;
}

bool ChannelInit::CreateStack(ChannelStackBuilder* builder) const {
  const auto& stack_config = stack_configs_[builder->channel_stack_type()];
  for (const auto& filter : stack_config.filters) {
    if (!filter.predicate(builder->channel_args())) continue;
    builder->AppendFilter(filter.filter);
  }
  bool found_terminator = false;
  for (const auto& terminator : stack_config.terminators) {
    if (!terminator.predicate(builder->channel_args())) continue;
    builder->AppendFilter(terminator.filter);
    found_terminator = true;
    break;
  }
  if (!found_terminator) return false;
  for (const auto& post_processor : stack_config.post_processors_) {
    post_processor(*builder);
  }
  return true;
}

}  // namespace grpc_core
