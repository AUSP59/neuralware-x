<!-- SPDX-License-Identifier: Apache-2.0 -->
# Runtime INT8 Quantization (Optional)

Enable quantized inference without changing on-disk bundles:
- CLI: add `--quantize_runtime`
- Server: set env `NWX_QUANTIZE=1`

This uses per-output-channel weight quantization and per-request dynamic activation scaling. Accumulates in int32, then rescales to float. It works on CPU (OpenMP parallel loops available) and coexists with BLAS/GPU paths.
