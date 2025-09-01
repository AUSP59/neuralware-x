<!-- SPDX-License-Identifier: Apache-2.0 -->
# Fuzzing

Build with Clang and libFuzzer:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DNWX_BUILD_FUZZERS=ON -DNWX_SANITIZE=ON
cmake --build build --target csv_fuzz --parallel
./build/csv_fuzz -max_total_time=30
```
