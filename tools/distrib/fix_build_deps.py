#!/usr/bin/env python3

# Copyright 2022 gRPC authors.
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

import argparse
import collections
from doctest import SKIP
from gc import collect
import os
import re
import subprocess
import sys
import tempfile

# find our home
ROOT = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), '../..'))
os.chdir(ROOT)

vendors = collections.defaultdict(list)
scores = collections.defaultdict(int)
avoidness = collections.defaultdict(int)
consumes = {}
no_update = set()
buildozer_commands = []
needs_codegen_base_src = set()
original_deps = {}
original_external_deps = {}

# TODO(ctiller): ideally we wouldn't hardcode a bunch of paths here.
# We can likely parse out BUILD files from dependencies to generate this index.
EXTERNAL_DEPS = {
    'absl/base/attributes.h':
        'absl/base:core_headers',
    'absl/base/call_once.h':
        'absl/base',
    # TODO(ctiller) remove this
    'absl/base/internal/endian.h':
        'absl-base',
    'absl/base/thread_annotations.h':
        'absl/base:core_headers',
    'absl/container/flat_hash_map.h':
        'absl/container:flat_hash_map',
    'absl/container/flat_hash_set.h':
        'absl/container:flat_hash_set',
    'absl/container/inlined_vector.h':
        'absl/container:inlined_vector',
    'absl/functional/bind_front.h':
        'absl/functional:bind_front',
    'absl/functional/function_ref.h':
        'absl/functional:function_ref',
    'absl/hash/hash.h':
        'absl/hash',
    'absl/memory/memory.h':
        'absl/memory',
    'absl/meta/type_traits.h':
        'absl/meta:type_traits',
    'absl/random/random.h':
        'absl/random',
    'absl/status/status.h':
        'absl/status',
    'absl/status/statusor.h':
        'absl/status:statusor',
    'absl/strings/ascii.h':
        'absl/strings',
    'absl/strings/cord.h':
        'absl/strings:cord',
    'absl/strings/escaping.h':
        'absl/strings',
    'absl/strings/match.h':
        'absl/strings',
    'absl/strings/numbers.h':
        'absl/strings',
    'absl/strings/str_cat.h':
        'absl/strings',
    'absl/strings/str_format.h':
        'absl/strings:str_format',
    'absl/strings/str_join.h':
        'absl/strings',
    'absl/strings/str_replace.h':
        'absl/strings',
    'absl/strings/str_split.h':
        'absl/strings',
    'absl/strings/string_view.h':
        'absl/strings',
    'absl/strings/strip.h':
        'absl/strings',
    'absl/strings/substitute.h':
        'absl/strings',
    'absl/synchronization/mutex.h':
        'absl/synchronization',
    'absl/synchronization/notification.h':
        'absl/synchronization',
    'absl/time/clock.h':
        'absl/time',
    'absl/time/time.h':
        'absl/time',
    'absl/types/optional.h':
        'absl/types:optional',
    'absl/types/span.h':
        'absl/types:span',
    'absl/types/variant.h':
        'absl/types:variant',
    'absl/utility/utility.h':
        'absl/utility',
    'address_sorting/address_sorting.h':
        'address_sorting',
    'ares.h':
        'cares',
    'gmock/gmock.h':
        'gtest',
    'gtest/gtest.h':
        'gtest',
    'opencensus/trace/context_util.h':
        'opencensus-trace-context_util',
    'opencensus/trace/propagation/grpc_trace_bin.h':
        'opencensus-trace-propagation',
    'opencensus/tags/context_util.h':
        'opencensus-tags-context_util',
    'openssl/bio.h':
        'libssl',
    'openssl/bn.h':
        'libcrypto',
    'openssl/buffer.h':
        'libcrypto',
    'openssl/crypto.h':
        'libcrypto',
    'openssl/engine.h':
        'libcrypto',
    'openssl/err.h':
        'libcrypto',
    'openssl/evp.h':
        'libcrypto',
    'openssl/hmac.h':
        'libcrypto',
    'openssl/pem.h':
        'libcrypto',
    'openssl/rsa.h':
        'libcrypto',
    'openssl/sha.h':
        'libcrypto',
    'openssl/ssl.h':
        'libssl',
    'openssl/tls1.h':
        'libssl',
    'openssl/x509.h':
        'libcrypto',
    'openssl/x509v3.h':
        'libcrypto',
    're2/re2.h':
        're2',
    'upb/def.h':
        'upb_lib',
    'upb/json_encode.h':
        'upb_json_lib',
    'upb/text_encode.h':
        'upb_textformat_lib',
    'upb/def.hpp':
        'upb_reflection',
    'upb/upb.h':
        'upb_lib',
    'upb/upb.hpp':
        'upb_lib',
    'xxhash.h':
        'xxhash',
    'zlib.h':
        'madler_zlib',
}

INTERNAL_DEPS = {
    'google/rpc/status.upb.h':
        'google_rpc_status_upb',
    'google/protobuf/any.upb.h':
        'protobuf_any_upb',
    'google/protobuf/duration.upb.h':
        'protobuf_duration_upb',
    'google/protobuf/struct.upb.h':
        'protobuf_struct_upb',
    'google/protobuf/timestamp.upb.h':
        'protobuf_timestamp_upb',
    'google/protobuf/wrappers.upb.h':
        'protobuf_wrappers_upb',
    'src/proto/grpc/channelz/channelz.grpc.pb.h':
        '//src/proto/grpc/channelz:channelz_proto',
    'src/proto/grpc/core/stats.pb.h':
        '//src/proto/grpc/core:stats_proto',
    'src/proto/grpc/health/v1/health.upb.h':
        'grpc_health_upb',
    'src/proto/grpc/lb/v1/load_reporter.grpc.pb.h':
        '//src/proto/grpc/lb/v1:load_reporter_proto',
    'src/proto/grpc/lb/v1/load_balancer.upb.h':
        'grpc_lb_upb',
    'src/proto/grpc/reflection/v1alpha/reflection.grpc.pb.h':
        '//src/proto/grpc/reflection/v1alpha:reflection_proto',
    'src/proto/grpc/gcp/transport_security_common.upb.h':
        'alts_upb',
    'src/proto/grpc/gcp/altscontext.upb.h':
        'alts_upb',
    'src/proto/grpc/lookup/v1/rls.upb.h':
        'rls_upb',
    'src/proto/grpc/lookup/v1/rls_config.upb.h':
        'rls_config_upb',
    'src/proto/grpc/lookup/v1/rls_config.upbdefs.h':
        'rls_config_upbdefs',
    'src/proto/grpc/testing/xds/v3/csds.grpc.pb.h':
        '//src/proto/grpc/testing/xds/v3:csds_proto',
    'xds/data/orca/v3/orca_load_report.upb.h':
        'xds_orca_upb',
    'xds/service/orca/v3/orca.upb.h':
        'xds_orca_service_upb',
    'xds/type/v3/typed_struct.upb.h':
        'xds_type_upb',
}

SKIP_DEPS = {'google/api/expr/v1alpha1/syntax.upb.h'}


class FakeSelects:

    def config_setting_group(self, **kwargs):
        pass


def grpc_cc_library(name,
                    hdrs=[],
                    public_hdrs=[],
                    srcs=[],
                    select_deps=None,
                    tags=[],
                    deps=[],
                    external_deps=[],
                    **kwargs):
    if select_deps or 'nofixdeps' in tags or 'grpc-autodeps' not in tags:
        no_update.add(name)
    scores[name] = len(public_hdrs + hdrs)
    if 'avoid_dep' in tags:
        avoidness[name] += 10
    if 'nofixdeps' in tags:
        avoidness[name] += 1
    for hdr in hdrs + public_hdrs:
        vendors[hdr].append(name)
    inc = set()
    original_deps[name] = frozenset(deps)
    original_external_deps[name] = frozenset(external_deps)
    for src in hdrs + public_hdrs + srcs:
        for line in open(src):
            m = re.search(r'#include <(.*)>', line)
            if m:
                inc.add(m.group(1))
            m = re.search(r'#include "(.*)"', line)
            if m:
                inc.add(m.group(1))
            if 'grpc::g_glip' in line or 'grpc:g_core_codegen_interface' in line:
                needs_codegen_base_src.add(name)
    consumes[name] = list(inc)


def buildozer(cmd, target):
    buildozer_commands.append('%s|%s' % (cmd, target))


def buildozer_set_list(name, values, target, via=""):
    if not values:
        buildozer('remove %s' % name, target)
        return
    adjust = via if via else name
    buildozer('set %s %s' % (adjust, ' '.join('"%s"' % s for s in values)),
              target)
    if via:
        buildozer('remove %s' % name, target)
        buildozer('rename %s %s' % (via, name), target)


def score_edit_distance(proposed, existing):
    """Score a proposed change primarily by edit distance"""
    sum = 0
    for p in proposed:
        if p not in existing:
            sum += 1
    for e in existing:
        if e not in proposed:
            sum += 1
    return sum


def total_score(proposal):
    return sum(scores[dep] for dep in proposal)


def total_avoidness(proposal):
    return sum(avoidness[dep] for dep in proposal)


def score_list_size(proposed, existing):
    """Score a proposed change primarily by number of dependencies"""
    return len(proposed)


def score_list_size(proposed, existing):
    """Score a proposed change primarily by number of dependencies"""
    return len(proposed)


def score_best(proposed, existing):
    """Score a proposed change primarily by dependency score"""
    return 0


SCORERS = {
    'edit_distance': score_edit_distance,
    'list_size': score_list_size,
    'best': score_best,
}

parser = argparse.ArgumentParser(description='Fix build dependencies')
parser.add_argument('targets',
                    nargs='*',
                    default=[],
                    help='targets to fix (empty => all)')
parser.add_argument('--score',
                    type=str,
                    default='edit_distance',
                    help='scoring function to use: one of ' +
                    ', '.join(SCORERS.keys()))
args = parser.parse_args()

exec(
    open('BUILD', 'r').read(), {
        'load': lambda filename, *args: None,
        'licenses': lambda licenses: None,
        'package': lambda **kwargs: None,
        'exports_files': lambda files: None,
        'config_setting': lambda **kwargs: None,
        'selects': FakeSelects(),
        'python_config_settings': lambda **kwargs: None,
        'grpc_cc_library': grpc_cc_library,
        'select': lambda d: d["//conditions:default"],
        'grpc_upb_proto_library': lambda name, **kwargs: None,
        'grpc_upb_proto_reflection_library': lambda name, **kwargs: None,
        'grpc_generate_one_off_targets': lambda: None,
        'filegroup': lambda name, **kwargs: None,
    }, {})


# Keeps track of all possible sets of dependencies that could satify the
# problem. (models the list monad in Haskell!)
class Choices:

    def __init__(self):
        self.choices = set()
        self.choices.add(frozenset())

    def add_one_of(self, choices):
        if not choices:
            return
        new_choices = set()
        for append_choice in choices:
            for choice in self.choices:
                new_choices.add(choice.union([append_choice]))
        self.choices = new_choices

    def add(self, choice):
        self.add_one_of([choice])

    def remove(self, remove):
        new_choices = set()
        for choice in self.choices:
            new_choices.add(choice.difference([remove]))
        self.choices = new_choices

    def best(self, scorer):
        best = None
        final_scorer = lambda x: (total_avoidness(x), scorer(x), total_score(x))
        for choice in self.choices:
            if best is None or final_scorer(choice) < final_scorer(best):
                best = choice
        return best


error = False
for library in sorted(consumes.keys()):
    if library in no_update:
        continue
    if args.targets and library not in args.targets:
        continue
    hdrs = sorted(consumes[library])
    deps = Choices()
    external_deps = Choices()
    for hdr in hdrs:
        if hdr == 'src/core/lib/profiling/stap_probes.h':
            continue

        if hdr in INTERNAL_DEPS:
            deps.add(INTERNAL_DEPS[hdr])
            continue

        if hdr in vendors:
            deps.add_one_of(vendors[hdr])
            continue

        if 'include/' + hdr in vendors:
            deps.add_one_of(vendors['include/' + hdr])
            continue

        if '.' not in hdr:
            # assume a c++ system include
            continue

        if hdr in EXTERNAL_DEPS:
            external_deps.add(EXTERNAL_DEPS[hdr])
            continue

        if hdr.startswith('opencensus/'):
            trail = hdr[len('opencensus/'):]
            trail = trail[:trail.find('/')]
            external_deps.add('opencensus-' + trail)
            continue

        if hdr.startswith('envoy/'):
            path, file = os.path.split(hdr)
            file = file.split('.')
            path = path.split('/')
            dep = '_'.join(path[:-1] + [file[1]])
            deps.add(dep)
            continue

        if hdr.startswith('google/protobuf/') and not hdr.endswith('.upb.h'):
            external_deps.add('protobuf_headers')
            continue

        if '/' not in hdr:
            # assume a system include
            continue

        is_sys_include = False
        for sys_path in [
                'sys',
                'arpa',
                'netinet',
                'linux',
                'android',
                'mach',
                'net',
                'CoreFoundation',
        ]:
            if hdr.startswith(sys_path + '/'):
                is_sys_include = True
                break
        if is_sys_include:
            # assume a system include
            continue

        if hdr in SKIP_DEPS:
            continue

        print("# ERROR: can't categorize header: %s" % hdr)
        error = True

    if library in needs_codegen_base_src:
        deps.add('grpc++_codegen_base_src')

    deps.remove(library)

    deps = sorted(
        deps.best(lambda x: SCORERS[args.score](x, original_deps[library])))
    external_deps = sorted(
        external_deps.best(lambda x: SCORERS[args.score]
                           (x, original_external_deps[library])))
    target = ':' + library
    buildozer_set_list('external_deps', external_deps, target, via='deps')
    buildozer_set_list('deps', deps, target)

if buildozer_commands:
    ok_statuses = (0, 3)
    temp = tempfile.NamedTemporaryFile()
    open(temp.name, 'w').write('\n'.join(buildozer_commands))
    c = ['tools/distrib/buildozer.sh', '-f', temp.name]
    r = subprocess.call(c)
    if r not in ok_statuses:
        print('{} failed with status {}'.format(c, r))
        sys.exit(1)

if error:
    sys.exit(1)
