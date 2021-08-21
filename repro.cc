// Copyright 2021 gRPC authors.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <functional>
#include <utility>
#include "absl/synchronization/mutex.h"
#include "absl/container/flat_hash_set.h"

#include "absl/types/variant.h"
#include "absl/meta/type_traits.h"

#include "absl/status/status.h"
#include "absl/status/statusor.h"

// Atomically increment a counter only if the counter value is not zero.
// Returns true if increment took place; false if counter is zero.
template <typename T>
inline bool IncrementIfNonzero(std::atomic<T>* p) {
  T count = p->load(std::memory_order_acquire);
  do {
    // If zero, we are done (without an increment). If not, we must do a CAS
    // to maintain the contract: do not increment the counter if it is already
    // zero
    if (count == 0) {
      return false;
    }
  } while (!p->compare_exchange_weak(
      count, count + 1, std::memory_order_acq_rel, std::memory_order_acquire));
  return true;
}

namespace grpc_core {

template <typename R, typename F0>
R Switch(char idx, F0 f0) {
  switch (idx) {
    case 0:
      return f0();
  }
  abort();
}

template <typename R, typename F0, typename F1>
R Switch(char idx, F0 f0, F1 f1) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2>
R Switch(char idx, F0 f0, F1 f1, F2 f2) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19, typename F20>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19, F20 f20) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
    case 20:
      return f20();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19, typename F20, typename F21>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19, F20 f20, F21 f21) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
    case 20:
      return f20();
    case 21:
      return f21();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19, typename F20, typename F21, typename F22>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19, F20 f20, F21 f21, F22 f22) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
    case 20:
      return f20();
    case 21:
      return f21();
    case 22:
      return f22();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19, typename F20, typename F21, typename F22, typename F23>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19, F20 f20, F21 f21, F22 f22,
         F23 f23) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
    case 20:
      return f20();
    case 21:
      return f21();
    case 22:
      return f22();
    case 23:
      return f23();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19, typename F20, typename F21, typename F22, typename F23,
          typename F24>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19, F20 f20, F21 f21, F22 f22, F23 f23,
         F24 f24) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
    case 20:
      return f20();
    case 21:
      return f21();
    case 22:
      return f22();
    case 23:
      return f23();
    case 24:
      return f24();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19, typename F20, typename F21, typename F22, typename F23,
          typename F24, typename F25>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19, F20 f20, F21 f21, F22 f22, F23 f23,
         F24 f24, F25 f25) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
    case 20:
      return f20();
    case 21:
      return f21();
    case 22:
      return f22();
    case 23:
      return f23();
    case 24:
      return f24();
    case 25:
      return f25();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19, typename F20, typename F21, typename F22, typename F23,
          typename F24, typename F25, typename F26>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19, F20 f20, F21 f21, F22 f22, F23 f23,
         F24 f24, F25 f25, F26 f26) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
    case 20:
      return f20();
    case 21:
      return f21();
    case 22:
      return f22();
    case 23:
      return f23();
    case 24:
      return f24();
    case 25:
      return f25();
    case 26:
      return f26();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19, typename F20, typename F21, typename F22, typename F23,
          typename F24, typename F25, typename F26, typename F27>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19, F20 f20, F21 f21, F22 f22, F23 f23,
         F24 f24, F25 f25, F26 f26, F27 f27) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
    case 20:
      return f20();
    case 21:
      return f21();
    case 22:
      return f22();
    case 23:
      return f23();
    case 24:
      return f24();
    case 25:
      return f25();
    case 26:
      return f26();
    case 27:
      return f27();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19, typename F20, typename F21, typename F22, typename F23,
          typename F24, typename F25, typename F26, typename F27, typename F28>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19, F20 f20, F21 f21, F22 f22, F23 f23,
         F24 f24, F25 f25, F26 f26, F27 f27, F28 f28) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
    case 20:
      return f20();
    case 21:
      return f21();
    case 22:
      return f22();
    case 23:
      return f23();
    case 24:
      return f24();
    case 25:
      return f25();
    case 26:
      return f26();
    case 27:
      return f27();
    case 28:
      return f28();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19, typename F20, typename F21, typename F22, typename F23,
          typename F24, typename F25, typename F26, typename F27, typename F28,
          typename F29>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19, F20 f20, F21 f21, F22 f22, F23 f23,
         F24 f24, F25 f25, F26 f26, F27 f27, F28 f28, F29 f29) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
    case 20:
      return f20();
    case 21:
      return f21();
    case 22:
      return f22();
    case 23:
      return f23();
    case 24:
      return f24();
    case 25:
      return f25();
    case 26:
      return f26();
    case 27:
      return f27();
    case 28:
      return f28();
    case 29:
      return f29();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19, typename F20, typename F21, typename F22, typename F23,
          typename F24, typename F25, typename F26, typename F27, typename F28,
          typename F29, typename F30>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19, F20 f20, F21 f21, F22 f22, F23 f23,
         F24 f24, F25 f25, F26 f26, F27 f27, F28 f28, F29 f29, F30 f30) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
    case 20:
      return f20();
    case 21:
      return f21();
    case 22:
      return f22();
    case 23:
      return f23();
    case 24:
      return f24();
    case 25:
      return f25();
    case 26:
      return f26();
    case 27:
      return f27();
    case 28:
      return f28();
    case 29:
      return f29();
    case 30:
      return f30();
  }
  abort();
}

template <typename R, typename F0, typename F1, typename F2, typename F3,
          typename F4, typename F5, typename F6, typename F7, typename F8,
          typename F9, typename F10, typename F11, typename F12, typename F13,
          typename F14, typename F15, typename F16, typename F17, typename F18,
          typename F19, typename F20, typename F21, typename F22, typename F23,
          typename F24, typename F25, typename F26, typename F27, typename F28,
          typename F29, typename F30, typename F31>
R Switch(char idx, F0 f0, F1 f1, F2 f2, F3 f3, F4 f4, F5 f5, F6 f6, F7 f7,
         F8 f8, F9 f9, F10 f10, F11 f11, F12 f12, F13 f13, F14 f14, F15 f15,
         F16 f16, F17 f17, F18 f18, F19 f19, F20 f20, F21 f21, F22 f22, F23 f23,
         F24 f24, F25 f25, F26 f26, F27 f27, F28 f28, F29 f29, F30 f30,
         F31 f31) {
  switch (idx) {
    case 0:
      return f0();
    case 1:
      return f1();
    case 2:
      return f2();
    case 3:
      return f3();
    case 4:
      return f4();
    case 5:
      return f5();
    case 6:
      return f6();
    case 7:
      return f7();
    case 8:
      return f8();
    case 9:
      return f9();
    case 10:
      return f10();
    case 11:
      return f11();
    case 12:
      return f12();
    case 13:
      return f13();
    case 14:
      return f14();
    case 15:
      return f15();
    case 16:
      return f16();
    case 17:
      return f17();
    case 18:
      return f18();
    case 19:
      return f19();
    case 20:
      return f20();
    case 21:
      return f21();
    case 22:
      return f22();
    case 23:
      return f23();
    case 24:
      return f24();
    case 25:
      return f25();
    case 26:
      return f26();
    case 27:
      return f27();
    case 28:
      return f28();
    case 29:
      return f29();
    case 30:
      return f30();
    case 31:
      return f31();
  }
  abort();
}

}  // namespace grpc_core

namespace grpc_core {

// Given a bit count as an integer, vend as member type `Type` a type with
// exactly that number of bits. Undefined if that bit count is not available.
template <std::size_t kBits>
struct UintSelector;
template <>
struct UintSelector<8> {
  typedef uint8_t Type;
};
template <>
struct UintSelector<16> {
  typedef uint16_t Type;
};
template <>
struct UintSelector<32> {
  typedef uint32_t Type;
};
template <>
struct UintSelector<64> {
  typedef uint64_t Type;
};

// An unsigned integer of some number of bits.
template <std::size_t kBits>
using Uint = typename UintSelector<kBits>::Type;

// Given the total number of bits that need to be stored, choose the size of
// 'unit' for a BitSet... We'll use an array of units to store the total set.
// For small bit counts we are selective in the type to try and balance byte
// size and performance
// - the details will likely be tweaked into the future.
// Once we get over 96 bits, we just use uint64_t for everything.
constexpr std::size_t ChooseUnitBitsForBitSet(std::size_t total_bits) {
  return total_bits <= 8    ? 8
         : total_bits <= 16 ? 16
         : total_bits <= 24 ? 8
         : total_bits <= 32 ? 32
         : total_bits <= 48 ? 16
         : total_bits <= 64 ? 64
         : total_bits <= 96 ? 32
                            : 64;
}

// A BitSet that's configurable.
// Contains storage for kTotalBits, stored as an array of integers of size
// kUnitBits. e.g. to store 72 bits in 8 bit chunks, we'd say BitSet<72, 8>.
// Since most users shouldn't care about the size of unit used, we default
// kUnitBits to whatever is selected by ChooseUnitBitsForBitSet
template <std::size_t kTotalBits,
          std::size_t kUnitBits = ChooseUnitBitsForBitSet(kTotalBits)>
class BitSet {
  static constexpr std::size_t kUnits =
      (kTotalBits + kUnitBits - 1) / kUnitBits;

 public:
  // Initialize to all bits false
  constexpr BitSet() : units_{} {}

  // Set bit i to true
   void set(int i) {
    units_[unit_for(i)] |= mask_for(i);
  }

  // Set bit i to is_set
  void set(int i, bool is_set) {
    if (is_set) {
      set(i);
    } else {
      clear(i);
    }
  }

  // Set bit i to false
  void clear(int i) {
    units_[unit_for(i)] &= ~mask_for(i);
  }

  // Return true if bit i is set
 bool is_set(int i) const {
    return (units_[unit_for(i)] & mask_for(i)) != 0;
  }

  // Return true if all bits are set
  bool all() const {
    if (kTotalBits % kUnitBits == 0) {
      // kTotalBits is a multiple of kUnitBits ==> we can just check for all
      // ones in each unit.
      for (std::size_t i = 0; i < kUnits; i++) {
        if (units_[i] != all_ones()) return false;
      }
      return true;
    } else {
      // kTotalBits is not a multiple of kUnitBits ==> we need special handling
      // for checking partial filling of the last unit (since not all of its
      // bits are used!)
      for (std::size_t i = 0; i < kUnits - 1; i++) {
        if (units_[i] != all_ones()) return false;
      }
      return units_[kUnits - 1] == n_ones(kTotalBits % kUnitBits);
    }
  }

  // Return true if *no* bits are set.
  bool none() const {
    for (std::size_t i = 0; i < kUnits; i++) {
      if (units_[i] != 0) return false;
    }
    return true;
  }

 private:
  // Given a bit index, return which unit it's stored in.
  static constexpr std::size_t unit_for(std::size_t bit) {
    return bit / kUnitBits;
  }

  // Given a bit index, return a mask to access that bit within it's unit.
  static constexpr Uint<kUnitBits> mask_for(std::size_t bit) {
    return Uint<kUnitBits>{1} << (bit % kUnitBits);
  }

  // Return a value that is all ones
  static constexpr Uint<kUnitBits> all_ones() {
    return Uint<kUnitBits>(~Uint<kUnitBits>(0));
  }

  // Return a value with n bottom bits ones
  static constexpr Uint<kUnitBits> n_ones(std::size_t n) {
    return n == kUnitBits ? all_ones() : (Uint<kUnitBits>(1) << n) - 1;
  }

  // The set of units - kUnitBits sized integers that store kUnitBits bits!
  Uint<kUnitBits> units_[kUnits];
};

}  // namespace grpc_core


namespace grpc_core {

// Call the destructor of p without having to name the type of p.
template <typename T>
void Destruct(T* p) {
  p->~T();
}

// Call the constructor of p without having to name the type of p and forward
// any arguments
template <typename T, typename... Args>
void Construct(T* p, Args&&... args) {
  new (p) T(std::forward<Args>(args)...);
}

}  // namespace grpc_core

namespace grpc_core {

// To avoid accidentally creating context types, we require an explicit
// specialization of this template per context type. The specialization need
// not contain any members, only exist.
// The reason for avoiding this is that context types each use a thread local.
template <typename T>
struct ContextType;

namespace promise_detail {

template <typename T>
class Context : public ContextType<T> {
 public:
  explicit Context(T* p) : old_(current_) { current_ = p; }
  ~Context() { current_ = old_; }
  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  static T* get() { return current_; }

 private:
  T* const old_;
  static thread_local T* current_;
};

template <typename T>
thread_local T*
Context<T>::current_;

template <typename T, typename F>
class WithContext {
 public:
  WithContext(F f, T* context) : context_(context), f_(std::move(f)) {}

  decltype(std::declval<F>()()) operator()() {
    Context<T> ctx(context_);
    return f_();
  }

 private:
  T* context_;
  F f_;
};

}  // namespace promise_detail

// Retrieve the current value of a context.
template <typename T>
T* GetContext() {
  return promise_detail::Context<T>::get();
}

// Given a promise and a context, return a promise that has that context set.
template <typename T, typename F>
promise_detail::WithContext<T, F> WithContext(F f, T* context) {
  return promise_detail::WithContext<T, F>(f, context);
}

}

namespace grpc_core {

// A type that signals a Promise is still pending and not yet completed.
// Allows writing 'return Pending{}' and with automatic conversions gets
// upgraded to a Poll<> object.
struct Pending {
  constexpr bool operator==(Pending) const { return true; }
};

// The result of polling a Promise once.
//
// Can be either pending - the Promise has not yet completed, or ready -
// indicating that the Promise has completed AND should not be polled again.
template <typename T>
using Poll = absl::variant<Pending, T>;

// Variant of Poll that serves as a ready value
static constexpr size_t kPollReadyIdx = 1;

// PollTraits tells us whether a type is Poll<> or some other type, and is
// leveraged in the PromiseLike/PromiseFactory machinery to select the
// appropriate implementation of those concepts based upon the return type of a
// lambda, for example (via enable_if).
template <typename T>
struct PollTraits {
  static constexpr bool is_poll() { return false; }
};

template <typename T>
struct PollTraits<Poll<T>> {
  using Type = T;
  static constexpr bool is_poll() { return true; }
};

}  // namespace grpc_core

// A Promise is a callable object that returns Poll<T> for some T.
// Often when we're writing code that uses promises, we end up wanting to also
// deal with code that completes instantaneously - that is, it returns some T
// where T is not Poll.
// PromiseLike wraps any callable that takes no parameters and implements the
// Promise interface. For things that already return Poll, this wrapping does
// nothing. For things that do not return Poll, we wrap the return type in Poll.
// This allows us to write things like:
//   Seq(
//     [] { return 42; },
//     ...)
// in preference to things like:
//   Seq(
//     [] { return Poll<int>(42); },
//     ...)
// or:
//   Seq(
//     [] -> Poll<int> { return 42; },
//     ...)
// leading to slightly more concise code and eliminating some rules that in
// practice people find hard to deal with.

namespace grpc_core {
namespace promise_detail {

template <typename T>
struct PollWrapper {
  static Poll<T> Wrap(T&& x) { return Poll<T>(std::forward<T>(x)); }
};

template <typename T>
struct PollWrapper<Poll<T>> {
  static Poll<T> Wrap(Poll<T>&& x) { return std::forward<Poll<T>>(x); }
};

template <typename T>
auto WrapInPoll(T&& x) -> decltype(PollWrapper<T>::Wrap(std::forward<T>(x))) {
  return PollWrapper<T>::Wrap(std::forward<T>(x));
}

template <typename F>
class PromiseLike {
 private:
  [[no_unique_address]] F f_;

 public:
  // NOLINTNEXTLINE - internal detail that drastically simplifies calling code.
  PromiseLike(F&& f) : f_(std::forward<F>(f)) {}
  auto operator()() -> decltype(WrapInPoll(f_())) { return WrapInPoll(f_()); }
  using Result = typename PollTraits<decltype(WrapInPoll(f_()))>::Type;
};

}  // namespace promise_detail
}  // namespace grpc_core

// PromiseFactory is an adaptor class.
//
// Where a Promise is a thing that's polled periodically, a PromiseFactory
// creates a Promise. Within this Promise/Activity framework, PromiseFactory's
// then provide the edges for computation -- invoked at state transition
// boundaries to provide the new steady state.
//
// A PromiseFactory formally is f(A) -> Promise<T> for some types A & T.
// This get a bit awkward and inapproprate to write however, and so the type
// contained herein can adapt various kinds of callable into the correct form.
// Of course a callable of a single argument returning a Promise will see an
// identity translation. One taking no arguments and returning a Promise
// similarly.
//
// A Promise passed to a PromiseFactory will yield a PromiseFactory that
// returns just that Promise.
//
// Generalizing slightly, a callable taking a single argument A and returning a
// Poll<T> will yield a PromiseFactory that captures it's argument A and
// returns a Poll<T>.
//
// Since various consumers of PromiseFactory run either repeatedly through an
// overarching Promises lifetime, or just once, and we can optimize just once
// by moving the contents of the PromiseFactory, two factory methods are
// provided: Once, that can be called just once, and Repeated, that can (wait
// for it) be called Repeatedly.

namespace grpc_core {
namespace promise_detail {

// Helper trait: given a T, and T x, is calling x() legal?
template <typename T, typename Ignored = void>
struct IsVoidCallable {
  static constexpr bool value = false;
};
template <typename F>
struct IsVoidCallable<F, absl::void_t<decltype(std::declval<F>()())>> {
  static constexpr bool value = true;
};

// T -> T, const T& -> T
template <typename T>
using RemoveCVRef = absl::remove_cv_t<absl::remove_reference_t<T>>;

// Given F(A,B,C,...), what's the return type?
template <typename T, typename Ignored = void>
struct ResultOfT;
template <typename F, typename... Args>
struct ResultOfT<F(Args...),
                 absl::void_t<decltype(std::declval<RemoveCVRef<F>>()(
                     std::declval<Args>()...))>> {
  using T = decltype(std::declval<RemoveCVRef<F>>()(std::declval<Args>()...));
};

template <typename T>
using ResultOf = typename ResultOfT<T>::T;

// Captures the promise functor and the argument passed.
// Provides the interface of a promise.
template <typename F, typename Arg>
class Curried {
 public:
  Curried(F&& f, Arg&& arg)
      : f_(std::forward<F>(f)), arg_(std::forward<Arg>(arg)) {}
  using Result = decltype(std::declval<F>()(std::declval<Arg>()));
  Result operator()() { return f_(arg_); }

 private:
  [[no_unique_address]] F f_;
  [[no_unique_address]] Arg arg_;
};

// Promote a callable(A) -> T | Poll<T> to a PromiseFactory(A) -> Promise<T> by
// capturing A.
template <typename A, typename F>
absl::enable_if_t<!IsVoidCallable<ResultOf<F(A)>>::value,
                  PromiseLike<Curried<F, A>>>
PromiseFactoryImpl(F&& f, A&& arg) {
  return Curried<F, A>(std::forward<F>(f), std::forward<A>(arg));
}

// Promote a callable() -> T|Poll<T> to a PromiseFactory(A) -> Promise<T>
// by dropping the argument passed to the factory.
template <typename A, typename F>
absl::enable_if_t<!IsVoidCallable<ResultOf<F()>>::value,
                  PromiseLike<RemoveCVRef<F>>>
PromiseFactoryImpl(F f, A&&) {
  return PromiseLike<F>(std::move(f));
}

// Promote a callable() -> T|Poll<T> to a PromiseFactory() -> Promise<T>
template <typename F>
absl::enable_if_t<!IsVoidCallable<ResultOf<F()>>::value,
                  PromiseLike<RemoveCVRef<F>>>
PromiseFactoryImpl(F f) {
  return PromiseLike<F>(std::move(f));
}

// Given a callable(A) -> Promise<T>, name it a PromiseFactory and use it.
template <typename A, typename F>
absl::enable_if_t<IsVoidCallable<ResultOf<F(A)>>::value,
                  PromiseLike<decltype(std::declval<F>()(std::declval<A>()))>>
PromiseFactoryImpl(F&& f, A&& arg) {
  return f(std::forward<A>(arg));
}

// Given a callable() -> Promise<T>, promote it to a
// PromiseFactory(A) -> Promise<T> by dropping the first argument.
template <typename A, typename F>
absl::enable_if_t<IsVoidCallable<ResultOf<F()>>::value,
                  PromiseLike<decltype(std::declval<F>()())>>
PromiseFactoryImpl(F&& f, A&&) {
  return f();
}

// Given a callable() -> Promise<T>, name it a PromiseFactory and use it.
template <typename F>
absl::enable_if_t<IsVoidCallable<ResultOf<F()>>::value,
                  PromiseLike<decltype(std::declval<F>()())>>
PromiseFactoryImpl(F&& f) {
  return f();
};

template <typename A, typename F>
class PromiseFactory {
 private:
  [[no_unique_address]] F f_;

 public:
  using Arg = A;
  using Promise =
      decltype(PromiseFactoryImpl(std::move(f_), std::declval<A>()));

  explicit PromiseFactory(F f) : f_(std::move(f)) {}

  Promise Once(Arg&& a) {
    return PromiseFactoryImpl(std::move(f_), std::forward<Arg>(a));
  }

  Promise Repeated(Arg&& a) const {
    return PromiseFactoryImpl(f_, std::forward<Arg>(a));
  }
};

template <typename F>
class PromiseFactory<void, F> {
 private:
  [[no_unique_address]] F f_;

 public:
  using Arg = void;
  using Promise = decltype(PromiseFactoryImpl(std::move(f_)));

  explicit PromiseFactory(F f) : f_(std::move(f)) {}

  Promise Once() { return PromiseFactoryImpl(std::move(f_)); }

  Promise Repeated() const { return PromiseFactoryImpl(f_); }
};

}  // namespace promise_detail
}  // namespace grpc_core

// Helpers for dealing with absl::Status/StatusOr generically

namespace grpc_core {
namespace promise_detail {

// Convert with a move the input status to an absl::Status.
template <typename T>
absl::Status IntoStatus(absl::StatusOr<T>* status) {
  return std::move(status->status());
}

// Convert with a move the input status to an absl::Status.
inline absl::Status IntoStatus(absl::Status* status) {
  return std::move(*status);
}

}  // namespace promise_detail
}  // namespace grpc_core

namespace grpc_core {

// A Wakeable object is used by queues to wake activities.
class Wakeable {
 public:
  // Wake up the underlying activity.
  // After calling, this Wakeable cannot be used again.
  virtual void Wakeup() = 0;
  // Drop this wakeable without waking up the underlying activity.
  virtual void Drop() = 0;

 protected:
  inline virtual ~Wakeable() {}
};

// An owning reference to a Wakeable.
// This type is non-copyable but movable.
class Waker {
 public:
  explicit Waker(Wakeable* wakeable) : wakeable_(wakeable) {}
  Waker() : wakeable_(&unwakeable_) {}
  ~Waker() { wakeable_->Drop(); }
  Waker(const Waker&) = delete;
  Waker& operator=(const Waker&) = delete;
  Waker(Waker&& other) noexcept : wakeable_(other.wakeable_) {
    other.wakeable_ = &unwakeable_;
  }
  Waker& operator=(Waker&& other) noexcept {
    std::swap(wakeable_, other.wakeable_);
    return *this;
  }

  // Wake the underlying activity.
  void Wakeup() {
    wakeable_->Wakeup();
    wakeable_ = &unwakeable_;
  }

  template <typename H>
  friend H AbslHashValue(H h, const Waker& w) {
    return H::combine(std::move(h), w.wakeable_);
  }

  bool operator==(const Waker& other) const noexcept {
    return wakeable_ == other.wakeable_;
  }

 private:
  class Unwakeable final : public Wakeable {
   public:
    void Wakeup() final { abort(); }
    void Drop() final {}
  };

  Wakeable* wakeable_;
  static Unwakeable unwakeable_;
};

// An Activity tracks execution of a single promise.
// It executes the promise under a mutex.
// When the promise stalls, it registers the containing activity to be woken up
// later.
// The activity takes a callback, which will be called exactly once with the
// result of execution.
// Activity execution may be cancelled by simply deleting the activity. In such
// a case, if execution had not already finished, the done callback would be
// called with absl::CancelledError().
// Activity also takes a CallbackScheduler instance on which to schedule
// callbacks to itself in a lock-clean environment.
class Activity : private Wakeable {
 public:
  // Cancel execution of the underlying promise.
  virtual void Cancel() LOCKS_EXCLUDED(mu_) = 0;

  // Destroy the Activity - used for the type alias ActivityPtr.
  struct Deleter {
    void operator()(Activity* activity) {
      activity->Cancel();
      activity->Unref();
    }
  };

  // Fetch the size of the implementation of this activity.
  virtual size_t Size() = 0;

  // Wakeup the current threads activity - will force a subsequent poll after
  // the one that's running.
  static void WakeupCurrent() { current()->got_wakeup_during_run_ = true; }

  // Return the current activity.
  // Additionally:
  // - assert that there is a current activity (and catch bugs if there's not)
  // - indicate to thread safety analysis that the current activity is indeed
  //   locked
  // - back up that assertation with a runtime check in debug builds (it's
  //   prohibitively expensive in non-debug builds)
  static Activity* current() ABSL_ASSERT_EXCLUSIVE_LOCK(current()->mu_) {
#ifndef NDEBUG
    
    if (g_current_activity_ != nullptr) {
      g_current_activity_->mu_.AssertHeld();
    }
#endif
    return g_current_activity_;
  }

  // Produce an activity-owning Waker. The produced waker will keep the activity
  // alive until it's awoken or dropped.
  Waker MakeOwningWaker() {
    Ref();
    return Waker(this);
  }

  // Produce a non-owning Waker. The waker will own a small heap allocated weak
  // pointer to this activity. This is more suitable for wakeups that may not be
  // delivered until long after the activity should be destroyed.
  Waker MakeNonOwningWaker() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);

 protected:
  inline ~Activity() override {
    if (handle_) {
      DropHandle();
    }
  }

  // All promise execution occurs under this mutex.
  absl::Mutex mu_;

  // Check if this activity is the current activity executing on the current
  // thread.
  bool is_current() const { return this == g_current_activity_; }
  // Check if there is an activity executing on the current thread.
  static bool have_current() { return g_current_activity_ != nullptr; }
  // Check if we got an internal wakeup since the last time this function was
  // called.
  bool got_wakeup() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    return absl::exchange(got_wakeup_during_run_, false);
  }

  // Set the current activity at construction, clean it up at destruction.
  class ScopedActivity {
   public:
    explicit ScopedActivity(Activity* activity) {
      
      g_current_activity_ = activity;
    }
    ~ScopedActivity() { g_current_activity_ = nullptr; }
    ScopedActivity(const ScopedActivity&) = delete;
    ScopedActivity& operator=(const ScopedActivity&) = delete;
  };

  // Implementors of Wakeable::Wakeup should call this after the wakeup has
  // completed.
  void WakeupComplete() { Unref(); }

 private:
  class Handle;

  void Ref() { refs_.fetch_add(1, std::memory_order_relaxed); }
  void Unref() {
    if (1 == refs_.fetch_sub(1, std::memory_order_acq_rel)) {
      delete this;
    }
  }

  // Return a Handle instance with a ref so that it can be stored waiting for
  // some wakeup.
  Handle* RefHandle() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);
  // If our refcount is non-zero, ref and return true.
  // Otherwise, return false.
  bool RefIfNonzero();
  // Drop the (proved existing) wait handle.
  void DropHandle() EXCLUSIVE_LOCKS_REQUIRED(mu_);

  // Current refcount.
  std::atomic<uint32_t> refs_{1};
  // If wakeup is called during Promise polling, we raise this flag and repoll
  // until things settle out.
  bool got_wakeup_during_run_ ABSL_GUARDED_BY(mu_) = false;
  // Handle for long waits. Allows a very small weak pointer type object to
  // queue for wakeups while Activity may be deleted earlier.
  Handle* handle_ ABSL_GUARDED_BY(mu_) = nullptr;
  // Set during RunLoop to the Activity that's executing.
  // Being set implies that mu_ is held.
  static thread_local Activity* g_current_activity_;
};

// Owned pointer to one Activity.
using ActivityPtr = std::unique_ptr<Activity, Activity::Deleter>;

namespace promise_detail {

template <typename Context>
class ContextHolder {
 public:
  explicit ContextHolder(Context value) : value_(std::move(value)) {}
  Context* GetContext() { return &value_; }

 private:
  Context value_;
};

template <typename Context>
class ContextHolder<Context*> {
 public:
  explicit ContextHolder(Context* value) : value_(value) {}
  Context* GetContext() { return value_; }

 private:
  Context* value_;
};

template <typename... Contexts>
class EnterContexts : public promise_detail::Context<Contexts>... {
 public:
  explicit EnterContexts(Contexts*... contexts)
      : promise_detail::Context<Contexts>(contexts)... {}
};

// Implementation details for an Activity of an arbitrary type of promise.
template <class F, class CallbackScheduler, class OnDone, typename... Contexts>
class PromiseActivity final
    : public Activity,
      private promise_detail::ContextHolder<Contexts>... {
 public:
  using Factory = PromiseFactory<void, F>;
  PromiseActivity(F promise_factory, CallbackScheduler callback_scheduler,
                  OnDone on_done, Contexts... contexts)
      : Activity(),
        ContextHolder<Contexts>(std::move(contexts))...,
        callback_scheduler_(std::move(callback_scheduler)),
        on_done_(std::move(on_done)) {
    // Lock, construct an initial promise from the factory, and step it.
    // This may hit a waiter, which could expose our this pointer to other
    // threads, meaning we do need to hold this mutex even though we're still
    // constructing.
    mu_.Lock();
    auto status = Start(Factory(std::move(promise_factory)));
    mu_.Unlock();
    // We may complete immediately.
    if (status.has_value()) {
      on_done_(std::move(*status));
    }
  }

  ~PromiseActivity() override {
    // We shouldn't destruct without calling Cancel() first, and that must get
    // us to be done_, so we assume that and have no logic to destruct the
    // promise here.
    
  }

  size_t Size() override { return sizeof(*this); }

  void Cancel() final {
    bool was_done;
    {
      absl::MutexLock lock(&mu_);
      // Check if we were done, and flag done.
      was_done = done_;
      if (!done_) MarkDone();
    }
    // If we were not done, then call the on_done callback.
    if (!was_done) {
      on_done_(absl::CancelledError());
    }
  }

 private:
  // Wakeup this activity. Arrange to poll the activity again at a convenient
  // time: this could be inline if it's deemed safe, or it could be by passing
  // the activity to an external threadpool to run. If the activity is already
  // running on this thread, a note is taken of such and the activity is
  // repolled if it doesn't complete.
  void Wakeup() final {
    // If there's no active activity, we can just run inline.
    if (!Activity::have_current()) {
      Step();
      WakeupComplete();
      return;
    }
    // If there is an active activity, but hey it's us, flag that and we'll loop
    // in RunLoop (that's calling from above here!).
    if (Activity::is_current()) {
      WakeupCurrent();
      WakeupComplete();
      return;
    }
    // Can't safely run, so ask to run later.
    callback_scheduler_([this]() {
      this->Step();
      this->WakeupComplete();
    });
  }

  // Drop a wakeup
  void Drop() final { this->WakeupComplete(); }

  // Notification that we're no longer executing - it's ok to destruct the
  // promise.
  void MarkDone() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    
    done_ = true;
    Destruct(&promise_holder_.promise);
  }

  // In response to Wakeup, run the Promise state machine again until it
  // settles. Then check for completion, and if we have completed, call on_done.
  void Step() ABSL_LOCKS_EXCLUDED(mu_) {
    // Poll the promise until things settle out under a lock.
    mu_.Lock();
    if (done_) {
      // We might get some spurious wakeups after finishing.
      mu_.Unlock();
      return;
    }
    auto status = RunStep();
    mu_.Unlock();
    if (status.has_value()) {
      on_done_(std::move(*status));
    }
  }

  // The main body of a step: set the current activity, and any contexts, and
  // then run the main polling loop. Contained in a function by itself in order
  // to keep the scoping rules a little easier in Step().
  absl::optional<absl::Status> RunStep() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    ScopedActivity scoped_activity(this);
    EnterContexts<Contexts...> contexts(
        static_cast<ContextHolder<Contexts>*>(this)->GetContext()...);
    return StepLoop();
  }

  // Similarly to RunStep, but additionally construct the promise from a promise
  // factory before entering the main loop. Called once from the constructor.
  absl::optional<absl::Status> Start(Factory promise_factory)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    ScopedActivity scoped_activity(this);
    EnterContexts<Contexts...> contexts(
        static_cast<ContextHolder<Contexts>*>(this)->GetContext()...);
    Construct(&promise_holder_.promise, promise_factory.Once());
    return StepLoop();
  }

  // Until there are no wakeups from within and the promise is incomplete: poll
  // the promise.
  absl::optional<absl::Status> StepLoop() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    
    do {
      // Run the promise.
      
      auto r = promise_holder_.promise();
      if (auto* status = absl::get_if<kPollReadyIdx>(&r)) {
        // If complete, destroy the promise, flag done, and exit this loop.
        MarkDone();
        return IntoStatus(status);
      }
      // Continue looping til no wakeups occur.
    } while (got_wakeup());
    return {};
  }

  using Promise = typename Factory::Promise;
  // We wrap the promise in a union to allow control over the construction
  // simultaneously with annotating mutex requirements and noting that the
  // promise contained may not use any memory.
  union PromiseHolder {
    PromiseHolder() {}
    ~PromiseHolder() {}
    [[no_unique_address]] Promise promise;
  };
  [[no_unique_address]] PromiseHolder promise_holder_ ABSL_GUARDED_BY(mu_);
  // Schedule callbacks on some external executor.
  [[no_unique_address]] CallbackScheduler callback_scheduler_;
  // Callback on completion of the promise.
  [[no_unique_address]] OnDone on_done_;
  // Has execution completed?
  [[no_unique_address]] bool done_ ABSL_GUARDED_BY(mu_) = false;
};

}  // namespace promise_detail

// Given a functor that returns a promise (a promise factory), a callback for
// completion, and a callback scheduler, construct an activity.
template <typename Factory, typename CallbackScheduler, typename OnDone,
          typename... Contexts>
ActivityPtr MakeActivity(Factory promise_factory,
                         CallbackScheduler callback_scheduler, OnDone on_done,
                         Contexts... contexts) {
  return ActivityPtr(
      new promise_detail::PromiseActivity<Factory, CallbackScheduler, OnDone,
                                          Contexts...>(
          std::move(promise_factory), std::move(callback_scheduler),
          std::move(on_done), std::move(contexts)...));
}

// A callback scheduler that simply crashes
struct NoCallbackScheduler {
  template <typename F>
  void operator()(F) {
    abort();
  }
};

}  // namespace grpc_core

namespace grpc_core {
namespace promise_detail {

// This union can either be a functor, or the result of the functor (after
// mapping via a trait). Allows us to remember the result of one joined functor
// until the rest are ready.
template <typename Traits, typename F>
union Fused {
  explicit Fused(F&& f) : f(std::forward<F>(f)) {}
  explicit Fused(PromiseLike<F>&& f) : f(std::forward<PromiseLike<F>>(f)) {}
  ~Fused() {}
  // Wrap the functor in a PromiseLike to handle immediately returning functors
  // and the like.
  using Promise = PromiseLike<F>;
  [[no_unique_address]] Promise f;
  // Compute the result type: We take the result of the promise, and pass it via
  // our traits, so that, for example, TryJoin and take a StatusOr<T> and just
  // store a T.
  using Result = typename Traits::template ResultType<typename Promise::Result>;
  [[no_unique_address]] Result result;
};

// A join gets composed of joints... these are just wrappers around a Fused for
// their data, with some machinery as methods to get the system working.
template <typename Traits, size_t kRemaining, typename... Fs>
struct Joint : public Joint<Traits, kRemaining - 1, Fs...> {
  // The index into Fs for this Joint
  static constexpr size_t kIdx = sizeof...(Fs) - kRemaining;
  // The next join (the one we derive from)
  using NextJoint = Joint<Traits, kRemaining - 1, Fs...>;
  // From Fs, extract the functor for this joint.
  using F = typename std::tuple_element<kIdx, std::tuple<Fs...>>::type;
  // Generate the Fused type for this functor.
  using Fsd = Fused<Traits, F>;
  [[no_unique_address]] Fsd fused;
  // Figure out what kind of bitmask will be used by the outer join.
  using Bits = BitSet<sizeof...(Fs)>;
  // Initialize from a tuple of pointers to Fs
  explicit Joint(std::tuple<Fs*...> fs)
      : NextJoint(fs), fused(std::move(*std::get<kIdx>(fs))) {}
  // Copy: assume that the Fuse is still in the promise state (since it's not
  // legal to copy after the first poll!)
  Joint(const Joint& j) : NextJoint(j), fused(j.fused.f) {}
  // Move: assume that the Fuse is still in the promise state (since it's not
  // legal to move after the first poll!)
  Joint(Joint&& j) noexcept
      : NextJoint(std::forward<NextJoint>(j)), fused(std::move(j.fused.f)) {}
  // Destruct: check bits to see if we're in promise or result state, and call
  // the appropriate destructor. Recursively, call up through the join.
  void DestructAll(const Bits& bits) {
    if (!bits.is_set(kIdx)) {
      Destruct(&fused.f);
    } else {
      Destruct(&fused.result);
    }
    NextJoint::DestructAll(bits);
  }
  // Poll all joints up, and then call finally.
  template <typename F>
  auto Run(Bits* bits, F finally) -> decltype(finally()) {
    // If we're still in the promise state...
    if (!bits->is_set(kIdx)) {
      // Poll the promise
      auto r = fused.f();
      if (auto* p = absl::get_if<kPollReadyIdx>(&r)) {
        // If it's done, then ask the trait to unwrap it and store that result
        // in the Fused, and continue the iteration. Note that OnResult could
        // instead choose to return a value instead of recursing through the
        // iteration, in that case we continue returning the same result up.
        // Here is where TryJoin can escape out.
        return Traits::OnResult(
            std::move(*p), [this, bits, &finally](typename Fsd::Result result) {
              bits->set(kIdx);
              Destruct(&fused.f);
              Construct(&fused.result, std::move(result));
              return NextJoint::Run(bits, std::move(finally));
            });
      }
    }
    // That joint is still pending... we'll still poll the result of the joints.
    return NextJoint::Run(bits, std::move(finally));
  }
};

// Terminating joint... for each of the recursions, do the thing we're supposed
// to do at the end.
template <typename Traits, typename... Fs>
struct Joint<Traits, 0, Fs...> {
  explicit Joint(std::tuple<Fs*...>) {}
  Joint(const Joint&) {}
  Joint(Joint&&) noexcept {}
  template <typename T>
  void DestructAll(const T&) {}
  template <typename F>
  auto Run(BitSet<sizeof...(Fs)>*, F finally) -> decltype(finally()) {
    return finally();
  }
};

template <typename Traits, typename... Fs>
class BasicJoin {
 private:
  // How many things are we joining?
  static constexpr size_t N = sizeof...(Fs);
  // Bitset: if a bit is 0, that joint is still in promise state. If it's 1,
  // then the joint has a result.
  [[no_unique_address]] BitSet<N> state_;
  // The actual joints, wrapped in an anonymous union to give us control of
  // construction/destruction.
  union {
    [[no_unique_address]] Joint<Traits, sizeof...(Fs), Fs...> joints_;
  };

  // Access joint index I
  template <size_t I>
  Joint<Traits, sizeof...(Fs) - I, Fs...>* GetJoint() {
    return static_cast<Joint<Traits, sizeof...(Fs) - I, Fs...>*>(&joints_);
  }

  // The tuple of results of all our promises
  using Tuple = std::tuple<typename Fused<Traits, Fs>::Result...>;

  // Collect up all the results and construct a tuple.
  template <size_t... I>
  Tuple Finish(absl::index_sequence<I...>) {
    return Tuple(std::move(GetJoint<I>()->fused.result)...);
  }

 public:
  explicit BasicJoin(Fs&&... fs) : joints_(std::tuple<Fs*...>(&fs...)) {}
  BasicJoin& operator=(const BasicJoin&) = delete;
  // Copy a join - only available before polling.
  BasicJoin(const BasicJoin& other) {
    assert(other.state_.none());
    Construct(&joints_, other.joints_);
  }
  // Move a join - only available before polling.
  BasicJoin(BasicJoin&& other) noexcept {
    assert(other.state_.none());
    Construct(&joints_, std::move(other.joints_));
  }
  ~BasicJoin() { joints_.DestructAll(state_); }
  using Result = decltype(Traits::Wrap(std::declval<Tuple>()));
  // Poll the join
  Poll<Result> operator()() {
    // Poll the joints...
    return joints_.Run(&state_, [this]() -> Poll<Result> {
      // If all of them are completed, collect the results, and then ask our
      // traits to wrap them - allowing for example TryJoin to turn tuple<A,B,C>
      // into StatusOr<tuple<A,B,C>>.
      if (state_.all()) {
        return Traits::Wrap(Finish(absl::make_index_sequence<N>()));
      } else {
        return Pending();
      }
    });
  }
};

}  // namespace promise_detail
}  // namespace grpc_core

namespace grpc_core {
namespace promise_detail {

struct JoinTraits {
  template <typename T>
  using ResultType = absl::remove_reference_t<T>;
  template <typename T, typename F>
  static auto OnResult(T result, F kontinue)
      -> decltype(kontinue(std::move(result))) {
    return kontinue(std::move(result));
  }
  template <typename T>
  static T Wrap(T x) {
    return x;
  }
};

template <typename... Promises>
using Join = BasicJoin<JoinTraits, Promises...>;

}  // namespace promise_detail

/// Combinator to run all promises to completion, and return a tuple
/// of their results.
template <typename... Promise>
promise_detail::Join<Promise...> Join(Promise... promises) {
  return promise_detail::Join<Promise...>(std::move(promises)...);
}

}  // namespace grpc_core

namespace grpc_core {

// A Promise is any functor that takes no arguments and returns Poll<T>.
// Most of the time we just pass around the functor, but occasionally
// it pays to have a type erased variant, which we define here.
template <typename T>
using Promise = std::function<Poll<T>()>;

// Helper to execute a promise immediately and return either the result or
// nothing.
template <typename Promise>
auto NowOrNever(Promise promise)
    -> absl::optional<typename promise_detail::PromiseLike<Promise>::Result> {
  auto r = promise_detail::PromiseLike<Promise>(std::move(promise))();
  if (auto* p = absl::get_if<kPollReadyIdx>(&r)) {
    return std::move(*p);
  }
  return {};
}

// A promise that never completes.
template <typename T>
struct Never {
  Poll<T> operator()() { return Pending(); }
};

namespace promise_detail {
// A promise that immediately completes.
template <typename T>
class Immediate {
 public:
  explicit Immediate(T value) : value_(std::move(value)) {}

  Poll<T> operator()() { return std::move(value_); }

 private:
  T value_;
};

}  // namespace promise_detail

// Return \a value immediately
template <typename T>
promise_detail::Immediate<T> Immediate(T value) {
  return promise_detail::Immediate<T>(std::move(value));
}

// Typecheck that a promise returns the expected return type.
// usage: auto promise = WithResult<int>([]() { return 3; });
// NOTE: there are tests in promise_test.cc that are commented out because they
// should fail to compile. When modifying this code these should be uncommented
// and their miscompilation verified.
template <typename T, typename F>
auto WithResult(F f) ->
    typename std::enable_if<std::is_same<decltype(f()), Poll<T>>::value,
                            F>::type {
  return f;
}

}  // namespace grpc_core

namespace grpc_core {
namespace promise_detail {

// Helper for SeqState to evaluate some common types to all partial
// specializations.
template <template <typename> class Traits, typename FPromise, typename FNext>
struct SeqStateTypes {
  // Our current promise.
  using Promise = FPromise;
  // The result of our current promise.
  using PromiseResult = typename Promise::Result;
  // Traits around the result of our promise.
  using PromiseResultTraits = Traits<PromiseResult>;
  // Wrap the factory callable in our factory wrapper to deal with common edge
  // cases. We use the 'unwrapped type' from the traits, so for instance, TrySeq
  // can pass back a T from a StatusOr<T>.
  using Next = promise_detail::PromiseFactory<
      typename PromiseResultTraits::UnwrappedType, FNext>;
};

// One state in a sequence.
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
template <template <typename> class Traits, char I, typename... Fs>
struct SeqState {
  // The state evaluated before this state.
  using PriorState = SeqState<Traits, I - 1, Fs...>;
  // Initialization from callables.
  explicit SeqState(std::tuple<Fs*...> fs)
      : next_factory(std::move(*std::get<I + 1>(fs))) {
    new (&prior) PriorState(fs);
  }
  // Move constructor - assumes we're in the initial state (move prior) as it's
  // illegal to move a promise after polling it.
  SeqState(SeqState&& other) noexcept
      : prior(std::move(other.prior)),
        next_factory(std::move(other.next_factory)) {}
  // Copy constructor - assumes we're in the initial state (move prior) as it's
  // illegal to move a promise after polling it.
  SeqState(const SeqState& other)
      : prior(other.prior), next_factory(other.next_factory) {}
  // Empty destructor - we instead destruct the innards in BasicSeq manually
  // depending on state.
  ~SeqState() {}
  // Evaluate the current promise, next promise factory types for this state.
  // The current promise is the next promise from the prior state.
  // The next factory callable is from the callables passed in:
  // Fs[0] is the initial promise, Fs[1] is the state 0 next factory, Fs[2] is
  // the state 1 next factory, etc...
  using Types = SeqStateTypes<
      Traits, typename PriorState::Types::Next::Promise,
      typename std::tuple_element<I + 1, std::tuple<Fs...>>::type>;
  // Storage for either the current promise or the prior state.
  union {
    // If we're in the prior state.
    [[no_unique_address]] PriorState prior;
    // The callables representing our promise.
    [[no_unique_address]] typename Types::Promise current_promise;
  };
  // Storage for the next promise factory.
  [[no_unique_address]] typename Types::Next next_factory;
};

// Partial specialization of SeqState above for the first state - it has no
// prior state, so we take the first callable from the template arg list and use
// it as a promise.
template <template <typename> class Traits, typename... Fs>
struct SeqState<Traits, 0, Fs...> {
  // Initialization from callables.
  explicit SeqState(std::tuple<Fs*...> args)
      : current_promise(std::move(*std::get<0>(args))),
        next_factory(std::move(*std::get<1>(args))) {}
  // Move constructor - it's assumed we're in this state (see above).
  SeqState(SeqState&& other) noexcept
      : current_promise(std::move(other.current_promise)),
        next_factory(std::move(other.next_factory)) {}
  // Copy constructor - it's assumed we're in this state (see above).
  SeqState(const SeqState& other)
      : current_promise(other.current_promise),
        next_factory(other.next_factory) {}
  // Empty destructor - we instead destruct the innards in BasicSeq manually
  // depending on state.
  ~SeqState(){};
  // Evaluate the current promise, next promise factory types for this state.
  // Our callable is the first element of Fs, wrapped in PromiseLike to handle
  // some common edge cases. The next factory is the second element.
  using Types = SeqStateTypes<
      Traits,
      PromiseLike<typename std::tuple_element<0, std::tuple<Fs...>>::type>,
      typename std::tuple_element<1, std::tuple<Fs...>>::type>;
  [[no_unique_address]] typename Types::Promise current_promise;
  [[no_unique_address]] typename Types::Next next_factory;
};

// Helper to get a specific state index.
// Calls the prior state, unless it's this state, in which case it returns
// that.
template <char I, template <typename> class Traits, char J, typename... Fs>
struct GetSeqStateInner {
  static SeqState<Traits, I, Fs...>* f(SeqState<Traits, J, Fs...>* p) {
    return GetSeqStateInner<I, Traits, J - 1, Fs...>::f(&p->prior);
  }
};

template <char I, template <typename> class Traits, typename... Fs>
struct GetSeqStateInner<I, Traits, I, Fs...> {
  static SeqState<Traits, I, Fs...>* f(SeqState<Traits, I, Fs...>* p) {
    return p;
  }
};

template <char I, template <typename> class Traits, char J, typename... Fs>
absl::enable_if_t<I <= J, SeqState<Traits, I, Fs...>*> GetSeqState(
    SeqState<Traits, J, Fs...>* p) {
  return GetSeqStateInner<I, Traits, J, Fs...>::f(p);
}

template <template <typename> class Traits, char I, typename... Fs, typename T>
auto CallNext(SeqState<Traits, I, Fs...>* state, T&& arg) -> decltype(
    SeqState<Traits, I, Fs...>::Types::PromiseResultTraits::CallFactory(
        &state->next_factory, std::forward<T>(arg))) {
  return SeqState<Traits, I, Fs...>::Types::PromiseResultTraits::CallFactory(
      &state->next_factory, std::forward<T>(arg));
}

// A sequence under stome traits for some set of callables Fs.
// Fs[0] should be a promise-like object that yields a value.
// Fs[1..] should be promise-factory-like objects that take the value from the
// previous step and yield a promise. Note that most of the machinery in
// PromiseFactory exists to make it possible for those promise-factory-like
// objects to be anything that's convenient. Traits defines how we move from one
// step to the next. Traits sets up the wrapping and escape handling for the
// sequence. Promises return wrapped values that the trait can inspect and
// unwrap before passing them to the next element of the sequence. The trait can
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
template <template <typename> class Traits, typename... Fs>
class BasicSeq {
 private:
  // Number of states in the sequence - we'll refer to this some!
  static constexpr char N = sizeof...(Fs);

  // Current state.
  static_assert(N < 128, "Long sequence... please revisit BasicSeq");
  char state_ = 0;
  // The penultimate state contains all the preceding states too.
  using PenultimateState = SeqState<Traits, N - 2, Fs...>;
  // The final state is simply the final promise, which is the next promise from
  // the penultimate state.
  using FinalPromise = typename PenultimateState::Types::Next::Promise;
  union {
    [[no_unique_address]] PenultimateState penultimate_state_;
    [[no_unique_address]] FinalPromise final_promise_;
  };
  using FinalPromiseResult = typename FinalPromise::Result;
  using Result = typename Traits<FinalPromiseResult>::WrappedType;

  // Get a state by index.
  template <char I>
      absl::enable_if_t < I<N - 2, SeqState<Traits, I, Fs...>*> state() {
    return GetSeqState<I>(&penultimate_state_);
  }

  template <char I>
  absl::enable_if_t<I == N - 2, PenultimateState*> state() {
    return &penultimate_state_;
  }

  // Get the next state's promise.
  template <char I>
  auto next_promise() -> absl::enable_if_t<
      I != N - 2,
      decltype(&GetSeqState<I + 1>(static_cast<PenultimateState*>(nullptr))
                    ->current_promise)> {
    return &GetSeqState<I + 1>(&penultimate_state_)->current_promise;
  }

  template <char I>
  absl::enable_if_t<I == N - 2, FinalPromise*> next_promise() {
    return &final_promise_;
  }

  // Callable to advance the state to the next one after I given the result from
  // state I.
  template <char I>
  struct RunNext {
    BasicSeq* s;
    template <typename T>
    Poll<Result> operator()(T&& value) {
      auto* prior = s->state<I>();
      using StateType = absl::remove_reference_t<decltype(*prior)>;
      // Destroy the promise that just completed.
      Destruct(&prior->current_promise);
      // Construct the next promise by calling the next promise factory.
      // We need to ask the traits to do this to deal with value
      // wrapping/unwrapping.
      auto n = StateType::Types::PromiseResultTraits::CallFactory(
          &prior->next_factory, std::forward<T>(value));
      // Now we also no longer need the factory, so we can destroy that.
      Destruct(&prior->next_factory);
      // Constructing the promise for the new state will use the memory
      // previously occupied by the promise & next factory of the old state.
      Construct(s->next_promise<I>(), std::move(n));
      // Store the state counter.
      s->state_ = I + 1;
      // Recursively poll the new current state.
      return s->RunState<I + 1>();
    }
  };

  // Poll the current state, advance it if necessary.
  template <char I>
  absl::enable_if_t<I != N - 1, Poll<Result>> RunState() {
    // Get a pointer to the state object.
    auto* s = state<I>();
    // Poll the current promise in this state.
    auto r = s->current_promise();
    // If we are still pending, say so by returning.
    if (absl::holds_alternative<Pending>(r)) {
      return Pending();
    }
    // Current promise is ready, as the traits to do the next thing.
    // That may be returning - eg if TrySeq sees an error.
    // Or it may be by calling the callable we hand down - RunNext - which
    // will advance the state and call the next promise.
    return Traits<
        typename absl::remove_reference_t<decltype(*s)>::Types::PromiseResult>::
        template CheckResultAndRunNext<Result>(
            std::move(absl::get<kPollReadyIdx>(std::move(r))),
            RunNext<I>{this});
  }

  // Specialization of RunState to run the final state.
  template <char I>
  absl::enable_if_t<I == N - 1, Poll<Result>> RunState() {
    // Poll the final promise.
    auto r = final_promise_();
    // If we are still pending, say so by returning.
    if (absl::holds_alternative<Pending>(r)) {
      return Pending();
    }
    // We are complete, return the (wrapped) result.
    return Result(std::move(absl::get<kPollReadyIdx>(std::move(r))));
  }

  // For state numbered I, destruct the current promise and the next promise
  // factory, and recursively destruct the next promise factories for future
  // states (since they also still exist).
  template <char I>
  absl::enable_if_t<I != N - 1, void>
  DestructCurrentPromiseAndSubsequentFactories() {
    Destruct(&GetSeqState<I>(&penultimate_state_)->current_promise);
    DestructSubsequentFactories<I>();
  }

  template <char I>
  absl::enable_if_t<I == N - 1, void>
  DestructCurrentPromiseAndSubsequentFactories() {
    Destruct(&final_promise_);
  }

  // For state I, destruct the next promise factory, and recursively the next
  // promise factories after.
  template <char I>
  absl::enable_if_t<I != N - 1, void> DestructSubsequentFactories() {
    Destruct(&GetSeqState<I>(&penultimate_state_)->next_factory);
    DestructSubsequentFactories<I + 1>();
  }

  template <char I>
  absl::enable_if_t<I == N - 1, void> DestructSubsequentFactories() {}

  // Placate older compilers by wrapping RunState in a struct so that their
  // parameter unpacking can work.
  template <char I>
  struct RunStateStruct {
    BasicSeq* s;
    Poll<Result> operator()() { return s->RunState<I>(); }
  };

  // Similarly placate those compilers for
  // DestructCurrentPromiseAndSubsequentFactories
  template <char I>
  struct DestructCurrentPromiseAndSubsequentFactoriesStruct {
    BasicSeq* s;
    void operator()() {
      return s->DestructCurrentPromiseAndSubsequentFactories<I>();
    }
  };

  // Run the current state (and possibly later states if that one finishes).
  // Single argument is a type that encodes the integer sequence 0, 1, 2, ...,
  // N-1 as a type, but which uses no memory. This is used to expand out
  // RunState instances using a template unpack to pass to Switch, which encodes
  // a switch statement over the various cases. This ultimately gives us a
  // Duff's device like mechanic for evaluating sequences.
  template <char... I>
  Poll<Result> Run(absl::integer_sequence<char, I...>) {
    return Switch<Poll<Result>>(state_, RunStateStruct<I>{this}...);
  }

  // Run the appropriate destructors for a given state.
  // Single argument is a type that encodes the integer sequence 0, 1, 2, ...,
  // N-1 as a type, but which uses no memory. This is used to expand out
  // DestructCurrentPromiseAndSubsequentFactories instances to pass to Switch,
  // which can choose the correct instance at runtime to destroy everything.
  template <char... I>
  void RunDestruct(absl::integer_sequence<char, I...>) {
    Switch<void>(
        state_, DestructCurrentPromiseAndSubsequentFactoriesStruct<I>{this}...);
  }

 public:
  // Construct a sequence given the callables that will control it.
  explicit BasicSeq(Fs... fs) : penultimate_state_(std::make_tuple(&fs...)) {}
  // No assignment... we don't need it (but if we ever find a good case, then
  // it's ok to implement).
  BasicSeq& operator=(const BasicSeq&) = delete;
  // Copy construction - only for state 0.
  // It's illegal to copy a Promise after polling it - if we are in state>0 we
  // *must* have been polled.
  BasicSeq(const BasicSeq& other) {
    assert(other.state_ == 0);
    new (&penultimate_state_) PenultimateState(other.penultimate_state_);
  }
  // Move construction - only for state 0.
  // It's illegal to copy a Promise after polling it - if we are in state>0 we
  // *must* have been polled.
  BasicSeq(BasicSeq&& other) noexcept {
    assert(other.state_ == 0);
    new (&penultimate_state_)
        PenultimateState(std::move(other.penultimate_state_));
  }
  // Destruct based on current state.
  ~BasicSeq() { RunDestruct(absl::make_integer_sequence<char, N>()); }

  // Poll the sequence once.
  Poll<Result> operator()() {
    return Run(absl::make_integer_sequence<char, N>());
  }
};

}  // namespace promise_detail
}  // namespace grpc_core

namespace grpc_core {

namespace promise_detail {

template <typename T>
struct SeqTraits {
  using UnwrappedType = T;
  using WrappedType = T;
  template <typename Next>
  static auto CallFactory(Next* next, T&& value)
      -> decltype(next->Once(std::forward<T>(value))) {
    return next->Once(std::forward<T>(value));
  }

  template <typename Result, typename PriorResult, typename RunNext>
  static Poll<Result> CheckResultAndRunNext(PriorResult prior,
                                            RunNext run_next) {
    return run_next(std::move(prior));
  }
};

template <typename... Fs>
using Seq = BasicSeq<SeqTraits, Fs...>;

}  // namespace promise_detail

// Sequencing combinator.
// Run the first promise.
// Pass its result to the second, and run the returned promise.
// Pass its result to the third, and run the returned promise.
// etc
// Return the final value.
template <typename... Functors>
promise_detail::Seq<Functors...> Seq(Functors... functors) {
  return promise_detail::Seq<Functors...>(std::move(functors)...);
}

template <typename F>
F Seq(F functor) {
  return functor;
}

}  // namespace grpc_core


namespace grpc_core {

// Helper type that can be used to enqueue many Activities waiting for some
// external state.
// Typically the external state should be guarded by mu_, and a call to
// WakeAllAndUnlock should be made when the state changes.
// Promises should bottom out polling inside pending(), which will register for
// wakeup and return Pending().
// Queues handles to Activities, and not Activities themselves, meaning that if
// an Activity is destroyed prior to wakeup we end up holding only a small
// amount of memory (around 16 bytes + malloc overhead) until the next wakeup
// occurs.
class WaitSet final {
  using WakerSet = absl::flat_hash_set<Waker>;

 public:
  // Register for wakeup, return Pending(). If state is not ready to proceed,
  // Promises should bottom out here.
  Pending AddPending(Waker waker) {
    pending_.emplace(std::move(waker));
    return Pending();
  }

  class WakeupSet {
   public:
    void Wakeup() {
      while (!wakeup_.empty()) {
        wakeup_.extract(wakeup_.begin()).value().Wakeup();
      }
    }

   private:
    friend class WaitSet;
    explicit WakeupSet(WakerSet&& wakeup)
        : wakeup_(std::forward<WakerSet>(wakeup)) {}
    WakerSet wakeup_;
  };

  [[nodiscard]] WakeupSet TakeWakeupSet() {
    return WakeupSet(std::move(pending_));
  }

 private:
  // Handles to activities that need to be awoken.
  WakerSet pending_;
};

}  // namespace grpc_core

using testing::_;
using testing::Mock;
using testing::MockFunction;
using testing::SaveArg;
using testing::StrictMock;

namespace grpc_core {

class MockCallbackScheduler {
 public:
  MOCK_METHOD(void, Schedule, (std::function<void()>));
};

// A simple Barrier type: stalls progress until it is 'cleared'.
class Barrier {
 public:
  struct Result {};

  Promise<Result> Wait() {
    return [this]() -> Poll<Result> {
      absl::MutexLock lock(&mu_);
      if (cleared_) {
        return Result{};
      } else {
        return wait_set_.AddPending(Activity::current()->MakeOwningWaker());
      }
    };
  }

  void Clear() {
    mu_.Lock();
    cleared_ = true;
    auto wakeup = wait_set_.TakeWakeupSet();
    mu_.Unlock();
    wakeup.Wakeup();
  }

 private:
  absl::Mutex mu_;
  WaitSet wait_set_ ABSL_GUARDED_BY(mu_);
  bool cleared_ ABSL_GUARDED_BY(mu_) = false;
};

// A simple Barrier type: stalls progress until it is 'cleared'.
// This variant supports only a single waiter.
class SingleBarrier {
 public:
  struct Result {};

  Promise<Result> Wait() {
    return [this]() -> Poll<Result> {
      absl::MutexLock lock(&mu_);
      if (cleared_) {
        return Result{};
      } else {
        waker_ = Activity::current()->MakeOwningWaker();
        return Pending();
      }
    };
  }

  void Clear() {
    mu_.Lock();
    cleared_ = true;
    auto waker = std::move(waker_);
    mu_.Unlock();
    waker.Wakeup();
  }

 private:
  absl::Mutex mu_;
  Waker waker_ GUARDED_BY(mu_);
  bool cleared_ GUARDED_BY(mu_) = false;
};

TEST(ActivityTest, ImmediatelyCompleteWithSuccess) {
  StrictMock<MockFunction<void(absl::Status)>> on_done;
  EXPECT_CALL(on_done, Call(absl::OkStatus()));
  MakeActivity(
      [] { return [] { return absl::OkStatus(); }; }, NoCallbackScheduler(),
      [&on_done](absl::Status status) { on_done.Call(std::move(status)); });
}

TEST(ActivityTest, ImmediatelyCompleteWithFailure) {
  StrictMock<MockFunction<void(absl::Status)>> on_done;
  EXPECT_CALL(on_done, Call(absl::CancelledError()));
  MakeActivity(
      [] { return [] { return absl::CancelledError(); }; },
      NoCallbackScheduler(),
      [&on_done](absl::Status status) { on_done.Call(std::move(status)); });
}

TEST(ActivityTest, DropImmediately) {
  StrictMock<MockFunction<void(absl::Status)>> on_done;
  EXPECT_CALL(on_done, Call(absl::CancelledError()));
  MakeActivity(
      [] { return []() -> Poll<absl::Status> { return Pending(); }; },
      NoCallbackScheduler(),
      [&on_done](absl::Status status) { on_done.Call(std::move(status)); });
}

TEST(ActivityTest, Cancel) {
  StrictMock<MockFunction<void(absl::Status)>> on_done;
  auto activity = MakeActivity(
      [] { return []() -> Poll<absl::Status> { return Pending(); }; },
      NoCallbackScheduler(),
      [&on_done](absl::Status status) { on_done.Call(std::move(status)); });
  EXPECT_CALL(on_done, Call(absl::CancelledError()));
  activity->Cancel();
  Mock::VerifyAndClearExpectations(&on_done);
  activity.reset();
}

template <typename B>
class BarrierTest : public testing::Test {
 public:
  using Type = B;
};

using BarrierTestTypes = testing::Types<Barrier, SingleBarrier>;
TYPED_TEST_SUITE(BarrierTest, BarrierTestTypes);

TYPED_TEST(BarrierTest, Barrier) {
  typename TestFixture::Type b;
  StrictMock<MockFunction<void(absl::Status)>> on_done;
  auto activity = MakeActivity(
      [&b] {
        return Seq(b.Wait(), [](typename TestFixture::Type::Result) {
          return absl::OkStatus();
        });
      },
      NoCallbackScheduler(),
      [&on_done](absl::Status status) { on_done.Call(std::move(status)); });
  // Clearing the barrier should let the activity proceed to return a result.
  EXPECT_CALL(on_done, Call(absl::OkStatus()));
  b.Clear();
}

TYPED_TEST(BarrierTest, BarrierPing) {
  typename TestFixture::Type b1;
  typename TestFixture::Type b2;
  StrictMock<MockFunction<void(absl::Status)>> on_done1;
  StrictMock<MockFunction<void(absl::Status)>> on_done2;
  MockCallbackScheduler scheduler;
  auto activity1 = MakeActivity(
      [&b1, &b2] {
        return Seq(b1.Wait(), [&b2](typename TestFixture::Type::Result) {
          // Clear the barrier whilst executing an activity
          b2.Clear();
          return absl::OkStatus();
        });
      },
      [&scheduler](std::function<void()> f) { scheduler.Schedule(f); },
      [&on_done1](absl::Status status) { on_done1.Call(std::move(status)); });
  auto activity2 = MakeActivity(
      [&b2] {
        return Seq(b2.Wait(), [](typename TestFixture::Type::Result) {
          return absl::OkStatus();
        });
      },
      [&scheduler](std::function<void()> f) { scheduler.Schedule(f); },
      [&on_done2](absl::Status status) { on_done2.Call(std::move(status)); });
  EXPECT_CALL(on_done1, Call(absl::OkStatus()));
  // Since barrier triggers inside activity1 promise, activity2 wakeup will be
  // scheduled from a callback.
  std::function<void()> cb;
  EXPECT_CALL(scheduler, Schedule(_)).WillOnce(SaveArg<0>(&cb));
  b1.Clear();
  Mock::VerifyAndClearExpectations(&on_done1);
  EXPECT_CALL(on_done2, Call(absl::OkStatus()));
  cb();
}

TYPED_TEST(BarrierTest, WakeSelf) {
  typename TestFixture::Type b;
  StrictMock<MockFunction<void(absl::Status)>> on_done;
  EXPECT_CALL(on_done, Call(absl::OkStatus()));
  MakeActivity(
      [&b] {
        return Seq(Join(b.Wait(),
                        [&b] {
                          b.Clear();
                          return 1;
                        }),
                   [](std::tuple<typename TestFixture::Type::Result, int>) {
                     return absl::OkStatus();
                   });
      },
      NoCallbackScheduler(),
      [&on_done](absl::Status status) { on_done.Call(std::move(status)); });
}

TYPED_TEST(BarrierTest, WakeAfterDestruction) {
  typename TestFixture::Type b;
  {
    StrictMock<MockFunction<void(absl::Status)>> on_done;
    EXPECT_CALL(on_done, Call(absl::CancelledError()));
    MakeActivity(
        [&b] {
          return Seq(b.Wait(), [](typename TestFixture::Type::Result) {
            return absl::OkStatus();
          });
        },
        NoCallbackScheduler(),
        [&on_done](absl::Status status) { on_done.Call(std::move(status)); });
  }
  b.Clear();
}

struct TestContext {
  bool* done;
};
template <>
struct ContextType<TestContext> {};

TEST(ActivityTest, WithContext) {
  bool done = false;
  StrictMock<MockFunction<void(absl::Status)>> on_done;
  EXPECT_CALL(on_done, Call(absl::OkStatus()));
  MakeActivity(
      [] {
        *GetContext<TestContext>()->done = true;
        return Immediate(absl::OkStatus());
      },
      NoCallbackScheduler(),
      [&on_done](absl::Status status) { on_done.Call(std::move(status)); },
      TestContext{&done});
  EXPECT_TRUE(done);
}

}  // namespace grpc_core

namespace grpc_core {

///////////////////////////////////////////////////////////////////////////////
// GLOBALS

thread_local Activity* Activity::g_current_activity_ = nullptr;
Waker::Unwakeable Waker::unwakeable_;

///////////////////////////////////////////////////////////////////////////////
// HELPER TYPES

// Weak handle to an Activity.
// Handle can persist while Activity goes away.
class Activity::Handle final : public Wakeable {
 public:
  explicit Handle(Activity* activity) : activity_(activity) {}

  // Ref the Handle (not the activity).
  void Ref() { refs_.fetch_add(1, std::memory_order_relaxed); }

  // Activity is going away... drop its reference and sever the connection back.
  void DropActivity() {
    mu_.Lock();
    activity_ = nullptr;
    mu_.Unlock();
    Unref();
  }

  // Activity needs to wake up (if it still exists!) - wake it up, and drop the
  // ref that was kept for this handle.
  void Wakeup() override ABSL_LOCKS_EXCLUDED(mu_) {
    mu_.Lock();
    // Note that activity refcount can drop to zero, but we could win the lock
    // against DropActivity, so we need to only increase activities refcount if
    // it is non-zero.
    if (activity_ && activity_->RefIfNonzero()) {
      Activity* activity = activity_;
      mu_.Unlock();
      // Activity still exists and we have a reference: wake it up, which will
      // drop the ref.
      activity->Wakeup();
    } else {
      // Could not get the activity - it's either gone or going. No need to wake
      // it up!
      mu_.Unlock();
    }
    // Drop the ref to the handle (we have one ref = one wakeup semantics).
    Unref();
  }

  void Drop() override { Unref(); }

 private:
  // Unref the Handle (not the activity).
  void Unref() {
    if (1 == refs_.fetch_sub(1, std::memory_order_acq_rel)) {
      delete this;
    }
  }

  // Two initial refs: one for the waiter that caused instantiation, one for the
  // activity.
  std::atomic<size_t> refs_{2};
  absl::Mutex mu_ ABSL_ACQUIRED_AFTER(activity_->mu_);
  Activity* activity_ ABSL_GUARDED_BY(mu_);
};

///////////////////////////////////////////////////////////////////////////////
// ACTIVITY IMPLEMENTATION

bool Activity::RefIfNonzero() { return IncrementIfNonzero(&refs_); }

Activity::Handle* Activity::RefHandle() {
  if (handle_ == nullptr) {
    // No handle created yet - construct it and return it.
    handle_ = new Handle(this);
    return handle_;
  } else {
    // Already had to create a handle, ref & return it.
    handle_->Ref();
    return handle_;
  }
}

void Activity::DropHandle() {
  handle_->DropActivity();
  handle_ = nullptr;
}

Waker Activity::MakeNonOwningWaker() { return Waker(RefHandle()); }

}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
