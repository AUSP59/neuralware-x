<!-- SPDX-License-Identifier: Apache-2.0 -->
# Build Tips

- Enable ccache locally for faster rebuilds:
  ```bash
  export CMAKE_CXX_COMPILER_LAUNCHER=ccache
  cmake -S . -B build -G Ninja
  ```
- Use toolchain files in `cmake/toolchains/*` to pin compilers.
