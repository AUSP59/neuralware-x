<!-- SPDX-License-Identifier: Apache-2.0 -->
# Observability

- Logs are structured JSON and include `severity`, and propagate `traceparent` (W3C).
- `/stats` exposes lightweight counters: requests & errors.
- Use `/metrics` for Prometheus metrics with histograms.
