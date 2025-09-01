<!-- SPDX-License-Identifier: Apache-2.0 -->
# Performance Tuning

- `NWX_BATCH_SIZE` and `NWX_BATCH_TIMEOUT_MS` for micro-batching.
- `NWX_THREADS` controls internal worker parallelism (future use).
- AVX2/FMA path is enabled by default. GPU/cuBLAS and INT8 runtime are opt-in as documented.
- Warmup run primes caches so the service reports `/ready` only after first forward succeeds.
