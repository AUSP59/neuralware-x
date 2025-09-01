<!-- SPDX-License-Identifier: Apache-2.0 -->
# Sanitizers

Enable AddressSanitizer + UBSan:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DNWX_SANITIZE=ON
cmake --build build --parallel
CTEST_OUTPUT_ON_FAILURE=1 ctest --test-dir build
```
