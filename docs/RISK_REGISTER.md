<!-- SPDX-License-Identifier: Apache-2.0 -->
# Risk Register (Living Document)

| ID | Risk | Likelihood | Impact | Mitigation | Owner | Status |
|----|------|------------|--------|------------|-------|--------|
| R-001 | Supply-chain compromise via dependency | Medium | High | SBOM, license gate, OSV/Trivy/Scorecard, pinning | Sec Team | Open |
| R-002 | ABI breakage between releases | Low | High | ABI diff gate, SOVERSION policy | Core | Open |
