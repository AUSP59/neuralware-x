<!-- SPDX-License-Identifier: Apache-2.0 -->
# Threat Model

## Assets
- Source integrity, releases, CI pipeline, distributed binaries.

## Actors
- Maintainers, contributors, CI, downstream users, attackers.

## Risks & Mitigations
- Supply chain: CodeQL, dependency review, SBOM, signed tags.
- Memory safety: sanitizer builds, hardening, warnings‑as‑errors CI.
- Reproducibility: deterministic seeds and fixed test datasets.
