#!/bin/bash

clang++ -fsanitize=memory -fsanitize-memory-use-after-dtor -fsanitize-memory-track-origins -o repro repro1.cc && MSAN_OPTIONS=poison_in_dtor=1 ./repro 2> log || true
set -ex
egrep 'SUMMARY: MemorySanitizer: use-of-uninitialized-value .* in grpc_core::Activity::~Activity()' log
egrep 'in __sanitizer_dtor_callback' log
egrep 'in grpc_core::promise_detail::PromiseActivity.*::~PromiseActivity()' log
