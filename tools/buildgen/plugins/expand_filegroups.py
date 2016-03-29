# Copyright 2015-2016, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Buildgen expand filegroups plugin.

This takes the list of libs from our yaml dictionary,
and expands any and all filegroup.

"""


def excluded(filename, exclude_res):
  for r in exclude_res:
    if r.search(filename):
      return True
  return False


FILEGROUP_LISTS = ['src', 'headers', 'public_headers']


def mako_plugin(dictionary):
  """The exported plugin code for expand_filegroups.

  The list of libs in the build.yaml file can contain "filegroups" tags.
  These refer to the filegroups in the root object. We will expand and
  merge filegroups on the src, headers and public_headers properties.

  """
  libs = dictionary.get('libs')
  filegroups_list = dictionary.get('filegroups')
  filegroups = {}

  todo = filegroups_list[:]
  skips = 0

  while todo:
    assert skips != len(todo), "infinite loop in filegroup uses clauses"
    # take the first element of the todo list
    cur = todo[0]
    todo = todo[1:]
    # check all uses filegroups are present (if no, skip and come back later)
    skip = False
    for uses in cur.get('uses', []):
      if uses not in filegroups:
        skip = True
    if skip:
      skips += 1
      todo.append(cur)
    else:
      skips = 0
      for uses in cur.get('uses', []):
        for lst in FILEGROUP_LISTS:
          vals = cur.get(lst, [])
          vals.extend(filegroups[uses].get(lst, []))
          cur[lst] = vals
      filegroups[cur['name']] = cur

  # the above expansion can introduce duplicate filenames: contract them here
  for fg in filegroups.itervalues():
    for lst in FILEGROUP_LISTS:
      fg[lst] = sorted(list(set(fg.get(lst, []))))

  for lib in libs:
    for fg_name in lib.get('filegroups', []):
      fg = filegroups[fg_name]

      for lst in FILEGROUP_LISTS:
        vals = lib.get(lst, [])
        vals.extend(fg.get(lst, []))
        lib[lst] = vals

    for lst in FILEGROUP_LISTS:
      lib[lst] = sorted(list(set(lib.get(lst, []))))
