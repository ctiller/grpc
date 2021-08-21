#!/bin/bash

bazel run --config=msan :repro &> log
egrep 'SUMMARY: MemorySanitizer: use-of-uninitialized-value .* in grpc_core::Activity::~Activity()' log

