#!/usr/bin/env python3

# Copyright 2023 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys

from mako.lookup import TemplateLookup
from mako.template import Template

tmpl_lookup = TemplateLookup(directories=["tools/codegen/core/templates/"])
seq_state = tmpl_lookup.get_template("seq_state.mako")
seq_state_types = tmpl_lookup.get_template("seq_state_types.mako")
seq_map = tmpl_lookup.get_template("seq_map.mako")
seq_factory_map = tmpl_lookup.get_template("seq_factory_map.mako")

front_matter = """
#ifndef GRPC_SRC_CORE_LIB_PROMISE_DETAIL_SEQ_STATE_H
#define GRPC_SRC_CORE_LIB_PROMISE_DETAIL_SEQ_STATE_H

// This file is generated by tools/codegen/core/gen_seq.py

#include <grpc/support/port_platform.h>

#include <stdint.h>

#include <utility>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/base/attributes.h"
#include "absl/strings/str_cat.h"

#include "src/core/lib/debug/trace.h"
#include "src/core/util/construct_destruct.h"
#include "src/core/util/debug_location.h"
#include "src/core/lib/promise/detail/promise_factory.h"
#include "src/core/lib/promise/detail/promise_like.h"
#include "src/core/lib/promise/poll.h"
#include "src/core/lib/promise/map.h"

// A sequence under some traits for some set of callables P, Fs.
// P should be a promise-like object that yields a value.
// Fs... should be promise-factory-like objects that take the value from the
// previous step and yield a promise. Note that most of the machinery in
// PromiseFactory exists to make it possible for those promise-factory-like
// objects to be anything that's convenient.
// Traits defines how we move from one step to the next. Traits sets up the
// wrapping and escape handling for the sequence.
// Promises return wrapped values that the trait can inspect and unwrap before
// passing them to the next element of the sequence. The trait can
// also interpret a wrapped value as an escape value, which terminates
// evaluation of the sequence immediately yielding a result. Traits for type T
// have the members:
//  * type UnwrappedType - the type after removing wrapping from T (i.e. for
//    TrySeq, T=StatusOr<U> yields UnwrappedType=U).
//  * type WrappedType - the type after adding wrapping if it doesn't already
//    exist (i.e. for TrySeq if T is not Status/StatusOr/void, then
//    WrappedType=StatusOr<T>; if T is Status then WrappedType=Status (it's
//    already wrapped!))
//  * template <typename Next> void CallFactory(Next* next_factory, T&& value) -
//    call promise factory next_factory with the result of unwrapping value, and
//    return the resulting promise.
//  * template <typename Result, typename RunNext> Poll<Result>
//    CheckResultAndRunNext(T prior, RunNext run_next) - examine the value of
//    prior, and decide to escape or continue. If escaping, return the final
//    sequence value of type Poll<Result>. If continuing, return the value of
//    run_next(std::move(prior)).
//
// A state contains the current promise, and the promise factory to turn the
// result of the current promise into the next state's promise. We play a shell
// game such that the prior state and our current promise are kept in a union,
// and the next promise factory is kept alongside in the state struct.
// Recursively this guarantees that the next functions get initialized once, and
// destroyed once, and don't need to be moved around in between, which avoids a
// potential O(n**2) loop of next factory moves had we used a variant of states
// here. The very first state does not have a prior state, and so that state has
// a partial specialization below. The final state does not have a next state;
// that state is inlined in BasicSeq since that was simpler to type.

namespace grpc_core {
namespace promise_detail {
template <template<typename> class Traits, typename P, typename... Fs>
struct SeqState;
template <template<typename> class Traits, typename P, typename... Fs>
struct SeqStateTypes;
"""

seq_map_decls = """
template <template<typename> class Traits, typename P, typename... F>
using SeqMapType = decltype(SeqMap<Traits>(std::declval<P>(), std::declval<F>()...));
template <template<typename> class Traits, typename Arg, typename... F>
using SeqFactoryMapType = decltype(SeqFactoryMap<Traits, Arg>(std::declval<F>()...));
"""

end_matter = """
}  // namespace promise_detail
}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_PROMISE_DETAIL_SEQ_STATE_H
"""


# utility: print a big comment block into a set of files
def put_banner(files, banner):
    for f in files:
        for line in banner:
            print("// %s" % line, file=f)
        print("", file=f)


with open(sys.argv[0]) as my_source:
    copyright = []
    for line in my_source:
        if line[0] != "#":
            break
    for line in my_source:
        if line[0] == "#":
            copyright.append(line)
            break
    for line in my_source:
        if line[0] != "#":
            break
        copyright.append(line)

copyright = [line[2:].rstrip() for line in copyright]


class FoldAccumulator(object):
    def __init__(self, f):
        self.type_args = []
        self.instantiate_args = []
        self.fold = [-1]
        self.f = f
        self.uses_types_alias = False

    def add(self, i, instantaneous):
        if instantaneous:
            self.fold.append(i)
        else:
            self._finish_fold()
            self.fold = [i]

    def _finish_fold(self):
        print(f"// fold={self.fold}", file=f)
        if len(self.fold) == 1:
            if self.fold[0] != -1:
                self.type_args.append(f"F{self.fold[0]}")
                self.instantiate_args.append(
                    f"std::forward<F{self.fold[0]}>(f{self.fold[0]})"
                )
            else:
                self.type_args.append("P")
                self.instantiate_args.append("std::forward<P>(p)")
        else:
            if self.fold[0] != -1:
                fs = ", ".join(f"F{i}" for i in self.fold)
                fwd_fs = ", ".join(
                    f"std::forward<F{i}>(f{i})" for i in self.fold
                )
                arg = f"typename Types::PromiseResultTraits{self.fold[0]}::UnwrappedType"
                self.uses_types_alias = True
                self.type_args.append(f"SeqFactoryMapType<Traits, {arg}, {fs}>")
                self.instantiate_args.append(
                    f"SeqFactoryMap<Traits, {arg}>({fwd_fs})"
                )
            else:
                fs = ", ".join(f"F{i}" for i in self.fold[1:])
                fwd_fs = ", ".join(
                    f"std::forward<F{i}>(f{i})" for i in self.fold[1:]
                )
                self.type_args.append(f"SeqMapType<Traits, P, {fs}>")
                self.instantiate_args.append(
                    f"SeqMap<Traits>(std::forward<P>(p), {fwd_fs})"
                )

    def finish(self):
        print(f"// final fold={self.fold}", file=f)
        if self.fold[0] != -1:
            self.type_args.append(f"F{self.fold[0]}")
            self.instantiate_args.append(
                f"std::forward<F{self.fold[0]}>(f{self.fold[0]})"
            )
        else:
            self.type_args.append("P")
            self.instantiate_args.append("std::forward<P>(p)")
        self.fold = self.fold[1:]
        if len(self.type_args) == 1:
            ret = f"{self.instantiate_args[0]}"
        else:
            ret = f"SeqState<Traits, {','.join(self.type_args)}>({','.join(self.instantiate_args)}, whence)"
        if len(self.fold):
            ret = f"SeqMap<Traits>({ret}, {','.join(f'std::forward<F{i}>(f{i})' for i in self.fold)})"
        return ret


def gen_opt_seq_state(n, f):
    typename_fs = ", ".join(f"typename F{i}" for i in range(n - 1))
    args_fs = ", ".join(f"F{i}&& f{i}" for i in range(n - 1))
    fs = ", ".join(f"F{i}" for i in range(n - 1))
    fwd_fs = ", ".join(f"std::forward<F{i}>(f{i})" for i in range(n - 1))
    if n > 7:
        print(
            f"template <template <typename> class Traits, typename P, {typename_fs}>",
            file=f,
        )
        print(
            f"GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION auto FoldSeqState(P&& p, {args_fs}, DebugLocation whence) {{",
            file=f,
        )
        print(
            f"  return SeqState<Traits, P, {fs}>(std::forward<P>(p), {fwd_fs}, whence);",
            file=f,
        )
        print("}", file=f)
        return

    print(
        f"template <template <typename> class Traits, uint32_t kInstantBits, typename P, {typename_fs}>",
        file=f,
    )
    print(
        f"GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION auto FoldSeqStateImpl(P&& p, {args_fs}, DebugLocation whence) {{",
        file=f,
    )
    text = []
    uses_types_alias = False
    for i in range(n - 1, 0, -1):
        mask = (1 << i) - 1
        text.append(
            f"  {'else' if i != n-1 else ''} if constexpr ((kInstantBits & {bin(mask)}) == {bin(mask)}) {{"
        )
        if i == n - 1:
            left = "std::forward<P>(p)"
        else:
            left = f"FoldSeqStateImpl<Traits, (kInstantBits >> {i})>(std::forward<P>(p), {','.join(f'std::forward<F{j}>(f{j})' for j in range(0, n-1-i))}, whence)"
        text.append(
            f"    return SeqMap<Traits>({left}, {','.join(f'std::forward<F{j}>(f{j})' for j in range(n-1-i, n-1))});"
        )
        text.append("  }")
    for i in range(n - 2, 1, -1):
        mask = ((1 << i) - 1) << (n - 1 - i)
        not_mask = (1 << (n - 1)) - 1 - mask
        text.append(
            f"  else if constexpr ((kInstantBits & {bin(mask)}) == {bin(mask)}) {{"
        )
        map = f"SeqMap<Traits>(std::forward<P>(p), {','.join(f'std::forward<F{j}>(f{j})' for j in range(n-1-i))})"
        rest = f"{','.join(f'std::forward<F{j}>(f{j})' for j in range(n-1-i, n-1))}"
        text.append(
            f"    return FoldSeqStateImpl<Traits, (kInstantBits & {bin(not_mask)})>({map}, {rest}); // {i}"
        )
        text.append("  }")
    for i in range(0, 1 << (n - 1)):
        if (i & 1) != 0:
            continue
        if (i & (1 << (n - 2))) and i != 0b10:
            continue
        acc = FoldAccumulator(f)
        for j in range(n - 1):
            acc.add(j, i & (1 << (n - 2 - j)))
        ret = acc.finish()
        text.append(f"  else if constexpr (kInstantBits == {bin(i)}) {{")
        text.append(f"    return {ret};")
        text.append("  }")
        if acc.uses_types_alias:
            uses_types_alias = True
    if uses_types_alias:
        text = [f"  using Types = SeqStateTypes<Traits, P, {fs}>;"] + text
    for line in text:
        print(line, file=f)
    print("}", file=f)

    print(
        f"template <template <typename> class Traits, typename P, {typename_fs}>",
        file=f,
    )
    print(
        f"GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION auto FoldSeqState(P&& p, {args_fs}, DebugLocation whence) {{",
        file=f,
    )
    print(f"  using Types = SeqStateTypes<Traits, P, {fs}>;", file=f)
    instant_bits = " | ".join(
        f"(Types::NextFactory{i}::kInstantaneousPromise? {1<<(n-2-i)} : 0)"
        for i in range(0, n - 1)
    )
    print(f"  static constexpr uint32_t kInstantBits = {instant_bits};", file=f)
    print(
        f"  return FoldSeqStateImpl<Traits, kInstantBits>(std::forward<P>(p), {fwd_fs}, whence);",
        file=f,
    )
    print("}", file=f)


def gen_opt_seq_state_v2(n, f):
    typename_fs = ", ".join(f"typename F{i}" for i in range(n - 1))
    args_fs = ", ".join(f"F{i}&& f{i}" for i in range(n - 1))
    fs = ", ".join(f"F{i}" for i in range(n - 1))
    fwd_fs = ", ".join(f"std::forward<F{i}>(f{i})" for i in range(n - 1))

    print(
        f"template <template <typename> class Traits, uint32_t kInstantBits, typename P, {typename_fs}>",
        file=f,
    )
    print(
        f"GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION auto FoldSeqStateImpl(P&& p, {args_fs}, DebugLocation whence) {{",
        file=f,
    )
    print("}", file=f)

    print(
        f"template <template <typename> class Traits, typename P, {typename_fs}>",
        file=f,
    )
    print(
        f"GPR_ATTRIBUTE_ALWAYS_INLINE_FUNCTION auto FoldSeqState(P&& p, {args_fs}, DebugLocation whence) {{",
        file=f,
    )
    print(f"  using Types = SeqStateTypes<Traits, P, {fs}>;", file=f)
    instant_bits = " | ".join(
        f"(Types::NextFactory{i}::kInstantaneousPromise? {1<<(n-2-i)} : 0)"
        for i in range(0, n - 1)
    )
    print(f"  static constexpr uint32_t kInstantBits = {instant_bits};", file=f)
    print(
        f"  return FoldSeqStateImpl<Traits, kInstantBits>(std::forward<P>(p), {fwd_fs}, whence);",
        file=f,
    )
    print("}", file=f)


with open("src/core/lib/promise/detail/seq_state.h", "w") as f:
    put_banner([f], copyright)
    print(front_matter, file=f)
    for n in range(2, 14):
        print(seq_state_types.render(n=n), file=f)
    for n in range(2, 14):
        print(seq_state.render(n=n), file=f)
    for n in range(2, 14):
        print(seq_map.render(n=n), file=f)
    for n in range(3, 14):
        print(seq_factory_map.render(n=n), file=f)
    print(seq_map_decls, file=f)
    print(file=f)
    for n in range(2, 14):
        gen_opt_seq_state(n, f)
    print(end_matter, file=f)
