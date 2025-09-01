<!-- SPDX-License-Identifier: Apache-2.0 -->
# Backup & Disaster Recovery

- **Stateless core**: binaries and images are reproducible; rebuild from source + SBOM.
- **Stateful integrations** (databases, queues): out-of-scope for core; provide runbooks in `docs/RUNBOOKS/` if added.
- **Artifacts**: retain release tarballs, SBOMs, signatures, and `SHA256SUMS` in immutable storage.
- **Restore test**: perform periodic restore drills with pinned versions.
