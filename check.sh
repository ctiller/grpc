#!/bin/bash

clang++ -fsanitize=memory -fsanitize-memory-use-after-dtor -o repro repro1.cc
MSAN_OPTIONS=poison_in_dtor=1 ./repro > log
egrep 'SUMMARY: MemorySanitizer: use-of-uninitialized-value .* in grpc_core::Activity::~Activity()' log

