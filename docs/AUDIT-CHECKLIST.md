<!-- SPDX-License-Identifier: Apache-2.0 -->
# Audit Checklist (Security/Quality/Compliance)

- [ ] LICENSE, NOTICE present and correct (Apache‑2.0)
- [ ] SECURITY.md and `/.well-known/security.txt`
- [ ] CI green across OS matrix
- [ ] CodeQL results reviewed
- [ ] Sanitizers pass locally
- [ ] Reproducible: `./tools/verify.sh` OK
- [ ] SBOM generated and published
- [ ] CHANGELOG updated, SemVer respected
- [ ] Docs updated (README, API, ARCHITECTURE, THREAT MODEL)
- [ ] Release artifacts signed and checksummed (process in RELEASE.md)
