/*
 * Copyright 2021 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Automatically generated by tools/codegen/core/gen_switch.py
 */

#ifndef GRPC_CORE_LIB_PROMISE_DETAIL_SWITCH_H
#define GRPC_CORE_LIB_PROMISE_DETAIL_SWITCH_H

#include <grpc/support/port_platform.h>

#include <stdlib.h>

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

#endif  // GRPC_CORE_LIB_PROMISE_DETAIL_SWITCH_H