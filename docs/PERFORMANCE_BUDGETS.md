<!-- SPDX-License-Identifier: Apache-2.0 -->

# Performance Budgets

- CLI startup: < 50ms (cold) on x86_64 Release
- p95 inference latency: define per-model and hardware target
- Memory peak: define upper bounds per operation
- Binary size: < 10 MB (strip, LTO on)
