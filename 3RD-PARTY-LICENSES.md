<!-- SPDX-License-Identifier: Apache-2.0 -->
# Third-Party Licenses and Attributions

This repository **does not vendor** third‑party source code. All source under `src/`, `include/`, and `tests/` carries SPDX headers and is covered by this project's `LICENSE` and `NOTICE` files.

Optional build/runtime integrations may use the following external software, each under its own license (not bundled here):

- **OpenSSL** — OpenSSL and SSLeay licenses (for TLS, optional).
- **BLAS / CBLAS** — various implementations (OpenBLAS, MKL, etc.), optional.
- **CUDA Toolkit / cuBLAS** — NVIDIA proprietary, optional.
- **gRPC / Protocol Buffers** — Apache-2.0 / BSD-3-Clause, optional.
- **zlib** — zlib license, optional (gzip support).

Users enabling these options must ensure compliance with the respective licenses in their environments.
