#!/usr/bin/env python2.7

# Copyright 2021 gRPC authors.
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


# utility: print a big comment block into a set of files
def put_banner(files, banner):
    for f in files:
        print >> f, '/*'
        for line in banner:
            print >> f, ' * %s' % line
        print >> f, ' */'
        print >> f


with open('src/core/lib/promise/detail/switch.h', 'w') as H:
    # copy-paste copyright notice from this file
    with open(sys.argv[0]) as my_source:
        copyright = []
        for line in my_source:
            if line[0] != '#':
                break
        for line in my_source:
            if line[0] == '#':
                copyright.append(line)
                break
        for line in my_source:
            if line[0] != '#':
                break
            copyright.append(line)
        put_banner([H], [line[2:].rstrip() for line in copyright])

    put_banner([H], ["Automatically generated by %s" % sys.argv[0]])

    print >> H, "#ifndef GRPC_CORE_LIB_PROMISE_DETAIL_SWITCH_H"
    print >> H, "#define GRPC_CORE_LIB_PROMISE_DETAIL_SWITCH_H"
    print >> H
    print >> H, "#include <stdlib.h>"
    print >> H
    print >> H, "namespace grpc_core {"

    for n in range(1, 33):
        print >> H
        print >> H, "template <typename R, %s> R Switch(char idx, %s) {" % (
            ", ".join("typename F%d" % i for i in range(0, n)),
            ", ".join("F%d f%d" % (i, i) for i in range(0, n)),
        )
        print >> H, "  switch (idx) {"
        for i in range(0, n):
            print >> H, "   case %d: return f%d();" % (i, i)
        print >> H, "  }"
        print >> H, "  abort();"
        print >> H, "}"

    print >> H
    print >> H, "}"
    print >> H
    print >> H, "#endif // GRPC_CORE_LIB_PROMISE_DETAIL_SWITCH_H"
