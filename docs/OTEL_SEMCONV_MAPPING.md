<!-- SPDX-License-Identifier: Apache-2.0 -->
# OpenTelemetry Semantic Conventions Mapping

- `service.name`: `neuralware-x`
- `service.version`: from `NWX_VERSION_*`
- HTTP server metrics: map to `http.server.*` and `http.server.duration` histogram.
- Traces: `server` span kind for inbound requests; `client` when calling downstream.
- Resource attributes: `deployment.environment` from config.
