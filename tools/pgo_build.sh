#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
# Build with profile-generate
cmake -S . -B build-pgo-gen -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-fprofile-generate"
cmake --build build-pgo-gen -j
# Run minimal workload
if [ -x build-pgo-gen/bin/nwx_app ]; then build-pgo-gen/bin/nwx_app --help >/dev/null || true; fi
# Rebuild with profile-use
cmake -S . -B build-pgo-use -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-fprofile-use -fprofile-correction"
cmake --build build-pgo-use -j
echo "PGO build complete at build-pgo-use/"
