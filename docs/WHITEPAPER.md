<!-- SPDX-License-Identifier: Apache-2.0 -->
# NEURALWARE‑X — Architecture & Rationale (Whitepaper)

This document explains the end‑to‑end system: C++ core (MLP inference with BLAS/GPU/INT8/AVX2),
the HTTP server (TLS/mTLS/JWT/JWKS, RFC7807, NDJSON, gzip, rate limiting, readiness/metrics, OpenAPI),
and the operational posture (Kubernetes hardening, SBOM/SLSA/Scorecard, Cosign, PDB/HPA/NetworkPolicy,
ServiceMonitor, distroless, non‑root). It also covers performance considerations and testing strategy.
