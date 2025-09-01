<!-- SPDX-License-Identifier: Apache-2.0 -->
# Threat Model

- **Assets**: model parameters, tokens/keys, request data, logs/metrics, container images.
- **Trust boundaries**: client ↔ edge LB ↔ nwx_serve ↔ model bundle storage.
- **Controls**: TLS 1.2+, optional mTLS, JWT HS256/RS256/JWKS with claims, rate‑limits per IP/token,
  hardened containers (non‑root, RO FS), K8s NetworkPolicy, SBOM & signing, SLSA provenance.
- **Risks & Mitigations**: DoS → rate limiting & limits; Token theft → redaction + short TTLs; Supply‑chain →
  SBOM + scanning + signature; Model drift/bias → `nwx_drift` & `nwx_fairness` tooling.
