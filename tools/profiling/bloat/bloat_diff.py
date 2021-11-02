#!/usr/bin/env python3
#
# Copyright 2017 gRPC authors.
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
import csv
import glob
import math
import multiprocessing
import os
import pathlib
import shutil
import subprocess
import sys

sys.path.append(
    os.path.join(os.path.dirname(sys.argv[0]), '..', '..', 'run_tests',
                 'python_utils'))
import check_on_pr

argp = argparse.ArgumentParser(description='Perform diff on microbenchmarks')

argp.add_argument('-d',
                  '--diff_base',
                  type=str,
                  help='Commit or branch to compare the current one to')

argp.add_argument('-j', '--jobs', type=int, default=multiprocessing.cpu_count())

args = argp.parse_args()

# the libraries for which check bloat difference is calculated
LIBS = [
    'libgrpc.so',
    'libgrpc++.so',
]


def _build(output_dir):
    """Perform the cmake build under the output_dir."""
    shutil.rmtree(output_dir, ignore_errors=True)
    subprocess.check_call('mkdir -p %s' % output_dir, shell=True, cwd='.')
    subprocess.check_call(
        'cmake -DgRPC_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo ..',
        shell=True,
        cwd=output_dir)
    subprocess.check_call('make -j%d' % args.jobs, shell=True, cwd=output_dir)


_build('bloat_diff_new')

if args.diff_base:
    where_am_i = subprocess.check_output(
        ['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip()
    # checkout the diff base (="old")
    subprocess.check_call(['git', 'checkout', args.diff_base])
    subprocess.check_call(['git', 'submodule', 'update'])
    try:
        _build('bloat_diff_old')
    finally:
        # restore the original revision (="new")
        subprocess.check_call(['git', 'checkout', where_am_i])
        subprocess.check_call(['git', 'submodule', 'update'])

pathlib.Path('bloaty-build').mkdir(exist_ok=True)
subprocess.check_call(
    ['cmake', '-G', 'Unix Makefiles', '../third_party/bloaty'],
    cwd='bloaty-build')
subprocess.check_call('make -j%d' % args.jobs, shell=True, cwd='bloaty-build')

text = ''
diff_size = 0
for lib in LIBS:
    text += '****************************************************************\n\n'
    text += lib + '\n\n'
    old_version = glob.glob('bloat_diff_old/%s' % lib)
    new_version = glob.glob('bloat_diff_new/%s' % lib)
    assert len(new_version) == 1
    cmd = 'bloaty-build/bloaty -d compileunits,symbols'
    if old_version:
        assert len(old_version) == 1
        text += subprocess.check_output('%s %s -- %s' %
                                        (cmd, new_version[0], old_version[0]),
                                        shell=True).decode()
        for filename in [old_version, new_version]:
            subprocess.check_call('strip %s' % filename[0], shell=True)
        sections = [x for x in csv.reader(
            subprocess.check_output('bloaty-build/bloaty --csv %s -- %s' %
                                    (old_version[0], new_version[0]),
                                    shell=True).decode().splitlines())]
        print(sections)
        for section in sections[1:]:
            diff_size += int(section[2])
    else:
        text += subprocess.check_output('%s %s' % (cmd, new_version[0]),
                                        shell=True).decode()
    text += '\n\n'

severity = int(
    math.copysign(max(0, math.log(abs(diff_size) / 1000, 10),
                      diff_size))) if diff_size != 0 else 0

print("SEVERITY: %d" % severity)

print(text)
check_on_pr.check_on_pr('Bloat Difference', '```\n%s\n```' % text)
check_on_pr.label_significance_on_pr('bloat', severity)
