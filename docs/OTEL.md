<!-- SPDX-License-Identifier: Apache-2.0 -->
# OpenTelemetry (OTLP/HTTP) — Minimal Exporter

Enable with:
```
export NWX_OTEL_EXPORTER_URL=http://otel-collector:4318/v1/traces
```
The server will emit a tiny trace JSON per HTTP request. **HTTP only** (no TLS) by design for simplicity; use a sidecar/agent for secure export.
