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

#ifndef GRPC_SRC_CORE_LIB_GLOBAL_MEMOIZE_TABLE_H
#define GRPC_SRC_CORE_LIB_GLOBAL_MEMOIZE_TABLE_H

#include "no_destruct.h"
#include "per_cpu.h"
#include "sync.h"

#include "src/core/lib/avl/avl.h"
#include "src/core/lib/gprpp/no_destruct.h"
#include "src/core/lib/gprpp/per_cpu.h"

namespace grpc_core {

template <typename T, typename Arg>
T MakeMemoizedFrom(Arg&& arg) {
  class Table {
   public:
    T Make(Arg arg) {
      auto& cpu_map = cpu_maps_.this_cpu();
      ReleasableMutexLock cpu_lock(cpu_map.mu);
      auto* p = cpu_map.map.Lookup(arg);
      if (p != nullptr) return *p;
      auto cpu_avl = cpu_map.map;
      cpu_lock.Release();
      ReleasableMutexLock authoritative_lock(authoritative_map_.mu);
      auto authoritative_avl = authoritative_map_.map;
      if (!authoritative_avl.SameIdentity(cpu_avl)) {
        p = authoritative_avl.Lookup(arg);
        if (p != nullptr) return *p;
      }
      T value(arg);
      authoritative_avl = authoritative_avl.Add(std::move(arg), value);
      authoritative_map_.map = authoritative_avl;
      for (auto& upd_cpu_map : cpu_maps_) {
        MutexLock upd_cpu_lock(upd_cpu_map.mu);
        upd_cpu_map.map = authoritative_avl;
      }
      return value;
    }

   private:
    struct Map {
      Mutex mu;
      AVL<Arg, T> map ABSL_GUARDED_BY(mu);
    };
    Map authoritative_map_;
    PerCpu<Map> cpu_maps_;
  };
  static NoDestruct<Table> table;
  return table.Make(std::forward<Arg>(arg));
}

}  // namespace grpc_core

#endif
