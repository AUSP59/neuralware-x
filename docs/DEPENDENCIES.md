<!-- SPDX-License-Identifier: Apache-2.0 -->
# Dependencies Overview

Core build requires: **CMake ≥ 3.20**, a modern C++ compiler (GCC, Clang, or MSVC), and a POSIX-like shell for scripts.

Optional toggles in `CMakeLists.txt`:

- `NWX_ENABLE_CUDA` — Requires CUDA Toolkit + cuBLAS (NVIDIA).
- `NWX_ENABLE_OPENSSL` — Enables TLS if OpenSSL is present.
- `NWX_USE_OPENMP` — Enables OpenMP if your compiler supports it.
- `NWX_ENABLE_FUZZ` — Builds libFuzzer targets with Clang.
- `NWX_ENABLE_TESTS` — Builds unit tests.
- `NWX_ENABLE_COVERAGE` — GCC/Clang coverage flags.
- `NWX_ENABLE_SANITIZERS` — Address/Undefined sanitizers (non‑MSVC).

No third‑party code is vendored in this repository.
