
#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
# Phase 1: build with profile generation
cmake -S . -B build-pgo-gen -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchains/clang_pgo_gen.cmake -DNWX_ENABLE_TESTS=ON
cmake --build build-pgo-gen -j
# Exercise binaries (use tests as a proxy for workload)
ctest --test-dir build-pgo-gen --output-on-failure || true
# Phase 2: rebuild using collected profiles
cmake -S . -B build-pgo-use -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchains/clang_pgo_use.cmake -DNWX_ENABLE_TESTS=ON
cmake --build build-pgo-use -j
echo "PGO build complete: ./build-pgo-use"
