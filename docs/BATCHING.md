<!-- SPDX-License-Identifier: Apache-2.0 -->
# Server-side Micro-batching

Enable with environment variables:
- `NWX_BATCH_SIZE` (e.g., 32) to batch up to N requests together.
- `NWX_BATCH_TIMEOUT_MS` (default 5 ms) to flush even if the batch isn't full.

The server aggregates rows from concurrent `/predict` calls and computes a single forward pass. Each request waits on a future and receives only its own rows.
