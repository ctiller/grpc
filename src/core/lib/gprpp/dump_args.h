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

#ifndef GRPC_SRC_CORE_LIB_GPRPP_DUMP_ARGS_H
#define GRPC_SRC_CORE_LIB_GPRPP_DUMP_ARGS_H

#include <ostream>
#include <vector>

#include "absl/functional/any_invocable.h"

namespace grpc_core {
namespace dump_args_detail {

// Helper function... just ignore the initializer list passed into it.
// Allows doing 'statements' via parameter pack expansion in C++11 - given
// template <typename... Ts>:
//  do_these_things({(foo<Ts>(), 1)});
// will execute foo<T>() for each T in Ts.
// In this example we also leverage the comma operator to make the resultant
// type of each statement be a consistant int so that C++ type deduction works
// as we'd like (note that in the expression (a, 1) in C++, the 'result' of the
// expression is the value after the right-most ',' -- in this case 1, with a
// executed as a side effect.
template <typename T>
void do_these_things(std::initializer_list<T>) {}

class DumpArgs {
 public:
  template <typename... Args>
  explicit DumpArgs(const char* arg_string, const Args&... args)
      : arg_string_(arg_string) {
    do_these_things(
        {AddDumper([a = &args](std::ostream& os) { os << *a; })...});
  }

  friend std::ostream& operator<<(std::ostream& os, const DumpArgs& args);

 private:
  int AddDumper(absl::AnyInvocable<void(std::ostream&) const> dumper) {
    arg_dumpers_.push_back(std::move(dumper));
    return 0;
  }

  const char* arg_string_;
  std::vector<absl::AnyInvocable<void(std::ostream&) const>> arg_dumpers_;
};

}  // namespace dump_args_detail
}  // namespace grpc_core

// Helper to print a list of variables and their values.
// Usage:
//   int a = 1;
//   int b = 2;
//   LOG(INFO) << GRPC_DUMP_ARGS(a, b)
// Output:
//   a = 1, b = 2
#define GRPC_DUMP_ARGS(...) \
  grpc_core::dump_args_detail::DumpArgs(#__VA_ARGS__, __VA_ARGS__)

#endif  // GRPC_SRC_CORE_LIB_GPRPP_DUMP_ARGS_H
