#!/usr/bin/env python2.7

import subprocess
import os
import sys
import datetime
import shutil
import json
import collections
import tabulate

root = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), '..', '..'))
os.chdir(root)

LIBS_TO_DIRS = {
  'core': ['src/core', 'test/core', 'include/grpc'],
  'c++': ['src/cpp', 'test/cpp', 'include/grpc++'],
  'python': ['src/python'],
  'c#': ['src/csharp'],
  'node': ['src/node'],
  'objective-c': ['src/objective-c'],
  'php': ['src/php'],
  'ruby': ['src/ruby'],
}

def run(cmd):
  print '$ %s' % cmd
  return subprocess.check_output(cmd, shell=True)

args = {
  'work': '/tmp/grpc-lines-over-time',
  'current_branch': run('git rev-parse --abbrev-ref HEAD').strip()
}

def month_year_iter(start_month, start_year, end_month, end_year):
    ym_start= 12*start_year + start_month - 1
    ym_end= 12*end_year + end_month - 1
    for ym in range(ym_start, ym_end):
        y, m = divmod(ym, 12)
        yield y, m+1

run('git clone . %(work)s' % args)
os.chdir(args['work'])
result = []
try:
  header = ['date'] + sorted(LIBS_TO_DIRS.keys())
  for year, month in month_year_iter(12, 2014, datetime.date.today().month, datetime.date.today().year):
    args['year'] = year
    args['month'] = month
    run('git checkout `git rev-list -n 1 --before="%(year)d-%(month)02d-01 00:00" %(current_branch)s`' % args)
    row = ['%(year)d-%(month)02d-01' % args]
    for lib in sorted(LIBS_TO_DIRS.keys()):
      dirs = LIBS_TO_DIRS[lib]
      args['dirs'] = ' '.join(dirs)
      try:
        js = json.loads(run('cloc %(dirs)s --json' % args))
        row.append(js['SUM']['code'])
      except:
        row.append(0)
    result.append(row)
finally:
  os.chdir(root)
  shutil.rmtree(args['work'])

print tabulate.tabulate(result, headers=header)
