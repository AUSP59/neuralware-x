<!-- SPDX-License-Identifier: Apache-2.0 -->
# Changelog

## 0.1.0
- Initial public release: functional CPU MLP (XOR-ready), CLI, tests, docs, CI, SBOM.

## 0.2.0
- Adam optimizer, mini-batches, early stopping, standardization, metrics.
- Predict mode, improved CLI and config.
- Exported CMake targets, Dockerfile, devcontainer, coverage & lint tools.
- SPDX headers, REUSE, privacy & support docs.

## 1.0.0 (LTS)
- Stratified split, macro/micro F1 & PR-AUC, Prometheus metrics on server.
- Stable C API, optional CUDA hooks, coverage thresholds, wheels workflow.

## 1.1.0 (EVEREST)
- Temperature scaling calibration with validation split; temperature persisted in bundle.
- Server: Bearer token auth, token-bucket rate limiting, /healthz and /readyz, Prometheus metrics retained.
- New tool: nwx_eval to compute metrics from dataset/models.
- CLI prints macro/micro F1, ROC-AUC and ECE.

## 1.2.0 (TITAN)
- Optional TLS (OpenSSL), per‑IP rate limiting, content length limits, socket timeouts, graceful shutdown.
- Kubernetes manifests + Helm chart; Nix flake; man pages; shell completions.
- New tools: nwx_check (dataset validator), nwx_bench (benchmarks).
- Dependabot config and expanded docs.

## 1.3.0 (COLOSSUS)
- Concurrency limit & hot-reload in server; JSON request support; structured logs.
- MkDocs site + GitHub Pages workflow; pkg-config; systemd unit.
- SBOM via Syft & vuln scan via Grype in CI; REUSE compliance; cppcheck.
- Load test (k6) and example clients (C++/Python).

## 1.4.0 (ZENITH)
- AdamW optimizer, cosine LR schedule with warmup, gradient clipping.
- Tools: nwx_cv (K-fold), nwx_importance (permutation), nwx_report (markdown report).
- Server: JWT HS256 (OpenSSL) and optional mTLS.
- Determinism test, release workflow, expanded docs.

## 2.0.0 (ABSOLUTE)
- Real CUDA/cuBLAS backend (opt-in): GPU matmul with device memory & copies.
- Python wheels via scikit-build-core; improved install of `nwxpy`.
- Prometheus histograms (latency/size) in server; docs expanded.

## 3.0.0 (SINGULAR)
- OpenAPI spec + /openapi.yaml endpoint; structured logs carry W3C traceparent.
- New tools: nwx_fairness (group fairness metrics) and nwx_drift (PSI drift).
- Distroless Dockerfile + GH Action to build/push container; clang-format config; stricter coverage.

## 3.1.0 (PARAGON)
- Server: **JWT RS256**, **mTLS client CA**, **SIGHUP** hot-reload; **per-token** rate limiting.
- Performance: **OpenMP** fallback parallelism when no BLAS/GPU.
- Build: **hardening flags** (FORTIFY, RELRO/now, PIE); stricter coverage.
- Deploy: K8s/Helm hardened `securityContext`; distroless runs as **non-root**.
- DX: `.clang-tidy` configuration.

## 3.2.0 (QUANTUM)
- Runtime INT8 quantization with dynamic activation scaling; enable via `--quantize_runtime` or `NWX_QUANTIZE=1`.
- OpenMP parallel path for INT8 GEMM; optional AVX2 toggle (future hook).
- Docs and CLI/server integration.

## 4.0.0 (AURORA)
- JWT claims validation (iss/aud/exp/nbf) with leeway.
- TLS hardening: minimum TLS1.2 and secure ciphers.
- K8s HPA & PDB; OSSF Scorecard; SLSA provenance workflow.
- Community: CODEOWNERS, issue/PR templates, CITATION.cff.
- Ops docs: RUNBOOK, SLOs, Postmortem template.
- Tools: nwx_impute (CSV imputer) and nwx_confusion (confusion matrix).

## 5.0.0 (HYPERION)
- AVX2/FMA accelerated double-precision matmul with runtime CPU detection.
- /buildinfo endpoint; privacy log redaction; optional CORS.
- Multi-arch container build (amd64, arm64).
- Tool: nwx_prof (inference profiler).
- Docs: AVX2.md, PRIVACY.md, DATA_RETENTION.md, BUILDINFO.md.

## 6.0.0 (POLARIS)
- KServe v2 REST compatibility endpoints; NDJSON output option.
- JWT RS256 via **JWKS file** (`--jwt_jwks`), plus HS256/RS256 and claims checks.
- CORS preflight (OPTIONS) and configurable headers; stricter security headers.
- Structured JSON logs aligned with OpenTelemetry fields.
- New `/modelcard` endpoint serving a model card if present.
- K8s: NetworkPolicy and ServiceMonitor manifests.
- Wheels via **cibuildwheel** for manylinux & macOS; `.pre-commit-config.yaml`.

## 7.0.0 (ORION)
- OTel-style structured logs (trace/span fields).
- New /stats endpoint; privacy-preserving counters.
- Windows support: Winsock initialization & cleanup.
- Fuzzing target for CSV reader and Sanitizers build options.
- Workflows: PyPI publish, container signing (cosign), SBOM generation, DCO enforcement.
- Docs: FUZZING.md, SANITIZERS.md, SUPPLY_CHAIN.md, WINDOWS.md, OBSERVABILITY.md.

## 8.0.0 (NOVA)
- NDJSON streaming output (`Accept: application/x-ndjson` or `?format=ndjson`).
- RFC7807 problem+json errors; stricter input validation.
- Optional gzip responses (zlib) when `NWX_GZIP=1` + `Accept-Encoding: gzip`.
- `/signature` endpoint with model I/O metadata; HSTS when TLS is enabled.
- IPv6 dual-stack bind; gradient*input explanations tool (`nwx_explain`).
- Coverage threshold: 85%.

## 9.0.0 (NEBULA)
- **Request IDs**: incoming `X-Request-Id` is echoed; if missing, generated server-side and returned.
- **ETag / If-None-Match**: caching for `GET /openapi.yaml`, `GET /modelcard`, and `GET /buildinfo` (weak ETags).
- **Security headers** tightened: `Permissions-Policy`, `Cross-Origin-Opener-Policy`, `Cross-Origin-Resource-Policy`.
- **Docs**: whitepaper, threat model, governance, ethics, sustainability, accessibility.

## 10.0.0 (ZEUS)
- Python bindings (ctypes) via shared library `nwx_capi` and package `neuralwarex`.
- SSE streaming endpoint `POST /predict/stream`.
- Optional AES-256-GCM encrypted bundles `.enc` with `nwx_encrypt` CLI.
- Docs: PYTHON.md, ENCRYPTION.md, SSE.md.

## 11.0.0 (ATLAS)
- **/healthz** and **/ready** endpoints for probes.
- Optional **OpenTelemetry OTLP/HTTP exporter** (logs/traces) via `NWX_OTEL_EXPORTER_URL=http://host:4318`.
- Optional **gRPC server** scaffold (OFF by default), with proto & build notes.
- **Seccomp** profile and Helm hardening notes.
- **vcpkg.json** & **CMakePresets.json** for reproducible builds.
- **Gitleaks** workflow, **Dependabot** and **Renovate** configuration.
- **E2E smoke** script for quick validation.

## 12.0.0 (PROMETHEUS)
- **Server-side micro-batching** for HTTP `/predict` with `NWX_BATCH_SIZE` and `NWX_BATCH_TIMEOUT_MS` (real batching with a worker thread).
- **Warmup on startup** and improved `/ready` once warmup completes.
- **Thread control**: `NWX_THREADS` sets worker parallelism where applicable.
- **gRPC streaming & batching**: bidirectional streaming and batch RPC (when built with gRPC).
- **Basic OTLP/HTTP metrics** export (counter for requests) via `NWX_OTEL_METRICS_URL=http://host:4318/v1/metrics`.
- Docs: `BATCHING.md`, `PERFORMANCE.md` updated `GRPC.md`.

## 14.0.0 (KRONOS)
- **Incoming gzip**: request decompression when `Content-Encoding: gzip` (if zlib is available).
- **Rate limiting** with **RFC 9238** response headers and `Retry-After`; window configured by `NWX_RATELIMIT="LIMIT,WINDOW_SECONDS"`.
- **Multiple tokens**: `NWX_TOKENS` (comma-separated) accepted in addition to `--token` env.
- **Prometheus /metrics** (text exposition) with basic counters and gauges.
- **Coverage** threshold raised to **90%** in CI.
- **Postman collection** added under `api/postman_collection.json` for quick testing.
