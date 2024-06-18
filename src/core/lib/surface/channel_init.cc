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

#include "src/core/lib/surface/channel_init.h"

#include <string.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <type_traits>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

#include <grpc/support/log.h>
#include <grpc/support/port_platform.h>

#include "src/core/lib/debug/trace.h"
#include "src/core/lib/gprpp/crash.h"
#include "src/core/lib/gprpp/sync.h"
#include "src/core/lib/surface/channel_stack_type.h"

namespace grpc_core {

UniqueTypeName (*NameFromChannelFilter)(const grpc_channel_filter*);

namespace {
struct CompareChannelFiltersByName {
  bool operator()(UniqueTypeName a, UniqueTypeName b) const {
    // Compare lexicographically instead of by pointer value so that different
    // builds make the same choices.
    return a.name() < b.name();
  }
};
}  // namespace

ChannelInit::FilterRegistration& ChannelInit::FilterRegistration::After(
    std::initializer_list<UniqueTypeName> filters) {
  for (auto filter : filters) {
    after_.push_back(filter);
  }
  return *this;
}

ChannelInit::FilterRegistration& ChannelInit::FilterRegistration::Before(
    std::initializer_list<UniqueTypeName> filters) {
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

ChannelInit::FilterRegistration& ChannelInit::FilterRegistration::IfNot(
    InclusionPredicate predicate) {
  predicates_.emplace_back(
      [predicate = std::move(predicate)](const ChannelArgs& args) {
        return !predicate(args);
      });
  return *this;
}

ChannelInit::FilterRegistration&
ChannelInit::FilterRegistration::IfHasChannelArg(const char* arg) {
  return If([arg](const ChannelArgs& args) { return args.Contains(arg); });
}

ChannelInit::FilterRegistration& ChannelInit::FilterRegistration::IfChannelArg(
    const char* arg, bool default_value) {
  return If([arg, default_value](const ChannelArgs& args) {
    return args.GetBool(arg).value_or(default_value);
  });
}

ChannelInit::FilterRegistration&
ChannelInit::FilterRegistration::ExcludeFromMinimalStack() {
  return If([](const ChannelArgs& args) { return !args.WantMinimalStack(); });
}

ChannelInit::FilterRegistration& ChannelInit::Builder::RegisterFilter(
    grpc_channel_stack_type type, UniqueTypeName name,
    const grpc_channel_filter* filter, FilterAdder filter_adder,
    SourceLocation registration_source) {
  filters_[type].emplace_back(std::make_unique<FilterRegistration>(
      name, filter, filter_adder, registration_source));
  return *filters_[type].back();
}

ChannelInit::StackConfig ChannelInit::BuildStackConfig(
    const std::vector<std::unique_ptr<ChannelInit::FilterRegistration>>&
        registrations,
    PostProcessor* post_processors, grpc_channel_stack_type type) {
  // Phase 1: Build a map from filter to the set of filters that must be
  // initialized before it.
  // We order this map (and the set of dependent filters) by filter name to
  // ensure algorithm ordering stability is deterministic for a given build.
  // We should not require this, but at the time of writing it's expected that
  // this will help overall stability.
  std::map<UniqueTypeName, FilterRegistration*> filter_to_registration;
  using DependencyMap =
      std::map<UniqueTypeName,
               std::set<UniqueTypeName, CompareChannelFiltersByName>,
               CompareChannelFiltersByName>;
  DependencyMap dependencies;
  std::vector<Filter> terminal_filters;
  for (const auto& registration : registrations) {
    if (filter_to_registration.count(registration->name_) > 0) {
      const auto first =
          filter_to_registration[registration->name_]->registration_source_;
      const auto second = registration->registration_source_;
      Crash(absl::StrCat("Duplicate registration of channel filter ",
                         registration->name_, "\nfirst: ", first.file(), ":",
                         first.line(), "\nsecond: ", second.file(), ":",
                         second.line()));
    }
    filter_to_registration[registration->name_] = registration.get();
    if (registration->terminal_) {
      CHECK(registration->after_.empty());
      CHECK(registration->before_.empty());
      CHECK(!registration->before_all_);
      CHECK_EQ(registration->ordering_, Ordering::kDefault);
      terminal_filters.emplace_back(
          registration->name_, registration->filter_, nullptr,
          std::move(registration->predicates_), registration->version_,
          registration->ordering_, registration->registration_source_);
    } else {
      dependencies[registration->name_];  // Ensure it's in the map.
    }
  }
  for (const auto& registration : registrations) {
    if (registration->terminal_) continue;
    CHECK_GT(filter_to_registration.count(registration->name_), 0u);
    for (UniqueTypeName after : registration->after_) {
      if (filter_to_registration.count(after) == 0) {
        gpr_log(
            GPR_DEBUG, "%s",
            absl::StrCat(
                "Filter ", after,
                " not registered, but is referenced in the after clause of ",
                registration->name_, " when building channel stack ",
                grpc_channel_stack_type_string(type))
                .c_str());
        continue;
      }
      dependencies[registration->name_].insert(after);
    }
    for (UniqueTypeName before : registration->before_) {
      if (filter_to_registration.count(before) == 0) {
        gpr_log(
            GPR_DEBUG, "%s",
            absl::StrCat(
                "Filter ", before,
                " not registered, but is referenced in the before clause of ",
                registration->name_, " when building channel stack ",
                grpc_channel_stack_type_string(type))
                .c_str());
        continue;
      }
      dependencies[before].insert(registration->name_);
    }
    if (registration->before_all_) {
      for (const auto& other : registrations) {
        if (other.get() == registration.get()) continue;
        if (other->terminal_) continue;
        dependencies[other->name_].insert(registration->name_);
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
          absl::StrAppend(&result, p.first, " ->");
          for (const auto& d : p.second) {
            absl::StrAppend(&result, " ", d);
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
    auto* registration = filter_to_registration[filter];
    filters.emplace_back(
        filter, registration->filter_, registration->filter_adder_,
        std::move(registration->predicates_), registration->version_,
        registration->ordering_, registration->registration_source_);
    for (auto& p : dependencies) {
      p.second.erase(filter);
    }
  }
  // Collect post processors that need to be applied.
  // We've already ensured the one-per-slot constraint, so now we can just
  // collect everything up into a vector and run it in order.
  std::vector<PostProcessor> post_processor_functions;
  for (int i = 0; i < static_cast<int>(PostProcessorSlot::kCount); i++) {
    if (post_processors[i] == nullptr) continue;
    post_processor_functions.emplace_back(std::move(post_processors[i]));
  }
  // Log out the graph we built if that's been requested.
  if (GRPC_TRACE_FLAG_ENABLED(channel_stack)) {
    // It can happen that multiple threads attempt to construct a core config at
    // once.
    // This is benign - the first one wins and others are discarded.
    // However, it messes up our logging and makes it harder to reason about the
    // graph, so we add some protection here.
    static Mutex* const m = new Mutex();
    MutexLock lock(m);
    // List the channel stack type (since we'll be repeatedly printing graphs in
    // this loop).
    LOG(INFO) << "ORDERED CHANNEL STACK "
              << grpc_channel_stack_type_string(type) << ":";
    // First build up a map of filter -> file:line: strings, because it helps
    // the readability of this log to get later fields aligned vertically.
    std::map<UniqueTypeName, std::string> loc_strs;
    size_t max_loc_str_len = 0;
    size_t max_filter_name_len = 0;
    auto add_loc_str = [&max_loc_str_len, &loc_strs, &filter_to_registration,
                        &max_filter_name_len](UniqueTypeName name) {
      max_filter_name_len = std::max(name.name().length(), max_filter_name_len);
      const auto registration =
          filter_to_registration[name]->registration_source_;
      absl::string_view file = registration.file();
      auto slash_pos = file.rfind('/');
      if (slash_pos != file.npos) {
        file = file.substr(slash_pos + 1);
      }
      auto loc_str = absl::StrCat(file, ":", registration.line(), ":");
      max_loc_str_len = std::max(max_loc_str_len, loc_str.length());
      loc_strs.emplace(name, std::move(loc_str));
    };
    for (const auto& filter : filters) {
      add_loc_str(filter.name);
    }
    for (const auto& terminal : terminal_filters) {
      add_loc_str(terminal.name);
    }
    for (auto& loc_str : loc_strs) {
      loc_str.second = absl::StrCat(
          loc_str.second,
          std::string(max_loc_str_len + 2 - loc_str.second.length(), ' '));
    }
    // For each regular filter, print the location registered, the name of the
    // filter, and if it needed to occur after some other filters list those
    // filters too.
    // Note that we use the processed after list here - earlier we turned Before
    // registrations into After registrations and we used those converted
    // registrations to build the final ordering.
    // If you're trying to track down why 'A' is listed as after 'B', look at
    // the following:
    //  - If A is registered with .After({B}), then A will be 'after' B here.
    //  - If B is registered with .Before({A}), then A will be 'after' B here.
    //  - If B is registered as BeforeAll, then A will be 'after' B here.
    for (const auto& filter : filters) {
      auto dep_it = original.find(filter.name);
      std::string after_str;
      if (dep_it != original.end() && !dep_it->second.empty()) {
        after_str = absl::StrCat(
            std::string(max_filter_name_len + 1 - filter.name.name().length(),
                        ' '),
            "after ",
            absl::StrJoin(dep_it->second, ", ",
                          [](std::string* out, UniqueTypeName name) {
                            out->append(std::string(name.name()));
                          }));
      } else {
        after_str =
            std::string(max_filter_name_len - filter.name.name().length(), ' ');
      }
      LOG(INFO) << "  " << loc_strs[filter.name] << filter.name << after_str
                << " [" << filter.ordering << "/" << filter.version << "]";
    }
    // Finally list out the terminal filters and where they were registered
    // from.
    for (const auto& terminal : terminal_filters) {
      const auto filter_str = absl::StrCat(
          "  ", loc_strs[terminal.name], terminal.name,
          std::string(max_filter_name_len + 1 - terminal.name.name().length(),
                      ' '),
          "[terminal]");
      LOG(INFO) << filter_str;
    }
  }
  // Check if there are no terminal filters: this would be an error.
  // GRPC_CLIENT_DYNAMIC stacks don't use this mechanism, so we don't check that
  // condition here.
  // Right now we only log: many tests end up with a core configuration that
  // is invalid.
  // TODO(ctiller): evaluate if we can turn this into a crash one day.
  // Right now it forces too many tests to know about channel initialization,
  // either by supplying a valid configuration or by including an opt-out flag.
  if (terminal_filters.empty() && type != GRPC_CLIENT_DYNAMIC) {
    gpr_log(
        GPR_ERROR,
        "No terminal filters registered for channel stack type %s; this is "
        "common for unit tests messing with CoreConfiguration, but will result "
        "in a ChannelInit::CreateStack that never completes successfully.",
        grpc_channel_stack_type_string(type));
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

bool ChannelInit::Filter::CheckPredicates(const ChannelArgs& args) const {
  for (const auto& predicate : predicates) {
    if (!predicate(args)) return false;
  }
  return true;
}

bool ChannelInit::CreateStack(ChannelStackBuilder* builder) const {
  const auto& stack_config = stack_configs_[builder->channel_stack_type()];
  for (const auto& filter : stack_config.filters) {
    if (SkipV2(filter.version)) continue;
    if (!filter.CheckPredicates(builder->channel_args())) continue;
    builder->AppendFilter(filter.filter);
  }
  int found_terminators = 0;
  for (const auto& terminator : stack_config.terminators) {
    if (!terminator.CheckPredicates(builder->channel_args())) continue;
    builder->AppendFilter(terminator.filter);
    ++found_terminators;
  }
  if (found_terminators != 1) {
    std::string error = absl::StrCat(
        found_terminators,
        " terminating filters found creating a channel of type ",
        grpc_channel_stack_type_string(builder->channel_stack_type()),
        " with arguments ", builder->channel_args().ToString(),
        " (we insist upon one and only one terminating "
        "filter)\n");
    if (stack_config.terminators.empty()) {
      absl::StrAppend(&error, "  No terminal filters were registered");
    } else {
      for (const auto& terminator : stack_config.terminators) {
        absl::StrAppend(&error, "  ", terminator.name, " registered @ ",
                        terminator.registration_source.file(), ":",
                        terminator.registration_source.line(), ": enabled = ",
                        terminator.CheckPredicates(builder->channel_args())
                            ? "true"
                            : "false",
                        "\n");
      }
    }
    LOG(ERROR) << error;
    return false;
  }
  for (const auto& post_processor : stack_config.post_processors) {
    post_processor(*builder);
  }
  return true;
}

void ChannelInit::AddToInterceptionChainBuilder(
    grpc_channel_stack_type type, InterceptionChainBuilder& builder) const {
  const auto& stack_config = stack_configs_[type];
  // Based on predicates build a list of filters to include in this segment.
  for (const auto& filter : stack_config.filters) {
    if (SkipV3(filter.version)) continue;
    if (!filter.CheckPredicates(builder.channel_args())) continue;
    if (filter.filter_adder == nullptr) {
      builder.Fail(absl::InvalidArgumentError(
          absl::StrCat("Filter ", filter.name, " has no v3-callstack vtable")));
      return;
    }
    filter.filter_adder(builder);
  }
}

}  // namespace grpc_core
