#!/usr/bin/env python3

import sys
import subprocess

files = subprocess.check_output(["git", "grep", "-l", sys.argv[1]]).decode("utf-8").splitlines()
for file in files:
    if file.startswith("src/core/lib/transport/metadata_batch."): continue
    text = open(file).read()
    prefix = "%s(" % sys.argv[1]
    out = ""
    while True:
        loc = text.find(prefix)
        if loc == -1: break
        out += text[:loc]
        text = text[loc + len(prefix):]
        depth = 1
        i = 0
        before_comma = None
        while depth != 0:
            if not before_comma and depth == 1 and text[i] == ",":
                before_comma = text[:i]
            if text[i] == "(": depth += 1
            elif text[i] == ")": depth -= 1
            i += 1
        if not before_comma:
            before_comma = text[:i-1]
            after_comma = ""
        else:
            after_comma = text[len(before_comma)+1:i-1]
        print(before_comma, " *** ", after_comma)
        before_comma = before_comma.strip()
        if before_comma.startswith("&"):
            before_comma = before_comma[1:]
            accessor = "."
        else:
            accessor = "->"
        repl = "%s%s%s(%s)" % (before_comma, accessor, sys.argv[2], after_comma)
        print(repl)
        out += repl
        text = text[i:]
    out += text
    open(file, "w").write(out)
