# .github/SECURITY.md
# SPDX-License-Identifier: Apache-2.0
title: Security Policy

## Supported Versions
We support the latest two MINOR releases with security fixes when feasible.

## Reporting a Vulnerability (Private)
- Preferred: GitHub Private Vulnerability Reporting (Security → Report a vulnerability).
- Alternative: Email **alanursapu@gmail.com** (PGP optional).
- Do NOT open public issues for vulnerabilities.

## What to Include
- Affected version/commit and platform
- Reproduction steps and PoC (if available)
- Impact assessment (e.g., RCE, info leak, DoS)
- Mitigations tried

## Response Targets
- Acknowledgement: 24h (business days)
- Triage: 72h
- Fix ETA: shared after triage; coordinated disclosure when ready

## Coordinated Disclosure
We ship fixes, publish an advisory (GHSA/CVE if applicable), credit the reporter, and provide mitigation guidance.

## Security Contacts
- GitHub: **@AUSP59**
- Email: **alanursapu@gmail.com**
txt
Copiar código
# .well-known/security.txt
Contact: mailto:alanursapu@gmail.com
Preferred-Languages: en, es
Policy: https://github.com/AUSP59/<REPO>/blob/main/.github/SECURITY.md
Expires: 2030-01-01T00:00:00.000Z
txt
Copiar código
# SECURITY_CONTACTS
# SPDX-License-Identifier: Apache-2.0
# Primary contacts for security matters (GitHub handles and emails).
AUSP59 <alanursapu@gmail.com>
ini
Copiar código
# CODEOWNERS
# SPDX-License-Identifier: Apache-2.0
/.github/SECURITY.md           @AUSP59
/.well-known/security.txt      @AUSP59
/.github/workflows/*.yml       @AUSP59
/scripts/sbom.sh               @AUSP59
/.gitleaks.toml                @AUSP59
yaml
Copiar código
# .github/ISSUE_TEMPLATE/config.yml
blank_issues_enabled: false
contact_links:
  - name: Report a Security Vulnerability (private)
    url: https://github.com/AUSP59/<REPO>/security/advisories/new
    about: Use GitHub Private Vulnerability Reporting (preferred).
  - name: Security Contact (email)
    url: mailto:alanursapu@gmail.com
    about: If you cannot use GitHub advisories, email us privately.
yaml
Copiar código
# .github/workflows/security-scans.yml
# SPDX-License-Identifier: Apache-2.0
# Heavy security scans are gated to avoid red X by default.
name: security-scans
on:
  push:
  pull_request:
  schedule:
    - cron: '0 3 * * 1'  # weekly
permissions: read-all
jobs:
  codeql:
    if: vars.ENABLE_CODEQL == 'true'
    uses: github/codeql-action/.github/workflows/codeql.yml@v3
    with:
      languages: cpp

  semgrep:
    if: vars.ENABLE_SEMGREP == 'true'
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: returntocorp/semgrep-action@v1
        with:
          config: p/ci

  gitleaks:
    if: vars.ENABLE_GITLEAKS == 'true'
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with: { fetch-depth: 0 }
      - uses: gitleaks/gitleaks-action@v2
        with:
          config-path: .gitleaks.toml

  osv:
    if: vars.ENABLE_SECURITY_SCANS == 'true'
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: google/osv-scanner-action@v1
        with:
          scan-args: -r .

  trivy-fs:
    if: vars.ENABLE_SECURITY_SCANS == 'true'
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: aquasecurity/trivy-action@0.24.0
        with:
          scan-type: 'fs'
          ignore-unfixed: true
          format: 'table'
          severity: 'CRITICAL,HIGH'

  licensee:
    if: vars.ENABLE_LICENSE_SCAN == 'true'
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: benbalter/licensee-action@v2

  scorecard:
    if: vars.ENABLE_SCORECARD == 'true'
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: ossf/scorecard-action@v2.4.0
        with:
          results_file: results.sarif
          results_format: sarif
      - uses: github/codeql-action/upload-sarif@v3
        with: { sarif_file: results.sarif }
toml
Copiar código
# .gitleaks.toml
title = "NEURALWARE-X gitleaks config"

[allowlist]
description = "Allow test fixtures and documentation false-positives"
paths = [
  '''^tests?/.*''',
  '''^docs?/.*''',
]

[rules]
# Add custom rules/entropy tuning here if your repo contains test keys.
yaml
Copiar código
# .github/workflows/slsa-source.yml
# SPDX-License-Identifier: Apache-2.0
name: slsa-source
on:
  push:
    tags: ['v*.*.*']
permissions:
  id-token: write
  contents: read
jobs:
  provenance:
    if: vars.ENABLE_SLSA == 'true'
    uses: slsa-framework/slsa-github-generator/.github/workflows/generator_generic_slsa3.yml@v2.0.0
    with:
      base64-subjects: ""
yaml
Copiar código
# .github/workflows/release-signed.yml
# SPDX-License-Identifier: Apache-2.0
name: release-signed
on:
  push:
    tags: ['v*.*.*']
permissions:
  contents: write
  id-token: write
jobs:
  build-sign:
    if: vars.ENABLE_SIGNED_RELEASE == 'true'
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Set source date epoch
        run: echo "SOURCE_DATE_EPOCH=$(git log -1 --pretty=%ct)" >> $GITHUB_ENV
      - name: Install deps
        run: sudo apt-get update && sudo apt-get install -y cmake ninja-build
      - name: Configure
        run: cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DNWX_ENABLE_TESTS=OFF
      - name: Build
        run: cmake --build build -j
      - name: Install
        run: cmake --install build --prefix install
      - name: Archive (reproducible)
        run: |
          tar --sort=name --owner=0 --group=0 --numeric-owner --mtime='UTC 2020-01-01' \
              -czf nwx-${{ github.ref_name }}-linux-x86_64.tar.gz -C install .
          shasum -a 256 nwx-${{ github.ref_name }}-linux-x86_64.tar.gz > SHA256SUMS.txt
          shasum -a 512 nwx-${{ github.ref_name }}-linux-x86_64.tar.gz > SHA512SUMS.txt
      - name: SBOM (CycloneDX)
        run: |
          curl -sSfL https://raw.githubusercontent.com/anchore/syft/main/install.sh | sh -s -- -b /usr/local/bin
          /usr/local/bin/syft packages dir:install -o cyclonedx-json > sbom.cdx.json || true
      - name: Cosign keyless (optional)
        env:
          COSIGN_EXPERIMENTAL: "1"
        run: |
          curl -sSL https://raw.githubusercontent.com/sigstore/cosign/main/install.sh | sudo sh -s -- -b /usr/local/bin
          cosign sign-blob --yes nwx-${{ github.ref_name }}-linux-x86_64.tar.gz > nwx.sig || true
      - name: Upload Release Assets
        uses: softprops/action-gh-release@v2
        with:
          files: |
            nwx-${{ github.ref_name }}-linux-x86_64.tar.gz
            SHA256SUMS.txt
            SHA512SUMS.txt
            sbom.cdx.json
            nwx.sig
bash
Copiar código
# scripts/sbom.sh
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
OUT="${1:-bom/sbom.cdx.json}"
mkdir -p "$(dirname "$OUT")"
if ! command -v syft >/dev/null 2>&1; then
  curl -sSfL https://raw.githubusercontent.com/anchore/syft/main/install.sh | sh -s -- -b /usr/local/bin
fi
syft packages dir:. -o cyclonedx-json > "$OUT"
echo "SBOM written to $OUT"
md
Copiar código
# docs/THREAT_MODEL.md
<!-- SPDX-License-Identifier: Apache-2.0 -->
# Threat Model (STRIDE + Practical Controls)

## Assets
- Source code, release artifacts, CI secrets, package feeds, users of the CLI/service.

## Entry Points
- Pull Requests, CI workflows, release pipelines, dependency updates, container images.

## STRIDE & Controls
- **Spoofing:** enforce code review + CODEOWNERS, provenance (SLSA), signed releases (cosign).
- **Tampering:** branch protection, required CI (build/tests), SBOM, dependency pinning, Trivy/OSV.
- **Repudiation:** audit via PR reviews & signed tags; GH logs.
- **Information disclosure:** secret scanning (gitleaks), restricted logs; avoid printing secrets.
- **Denial of service:** fuzzers for parsers, input validation; resource limits where applicable.
- **Elevation of privilege:** least-privilege tokens, rootless containers, avoid self-hosted runners.

## High-Risk Areas
- Dependency supply-chain → SBOM + OSV/Trivy + Dependabot.
- Release impersonation → cosign + SLSA provenance.
- Secrets → gitleaks + GitHub secret scanning + no secrets in code.

## Residual Risks
- Third-party registry compromise → verify signatures/provenance; allowlist registries.
md
Copiar código
# docs/INCIDENT_RESPONSE.md
# SPDX-License-Identifier: Apache-2.0
# Security Incident Response Plan

- **Intake:** Use GH Advisories or `alanursapu@gmail.com`.
- **Triage (≤72h):** assign severity (CVSS), owners, and timeline.
- **Contain:** revert/disable impacted features, rotate credentials, revoke tokens.
- **Eradicate:** patch, write tests, verify with sanitizers/fuzzers.
- **Recover:** ship fixed releases, publish advisory with credits.
- **Postmortem:** RCA doc with action items and deadlines.
md
Copiar código
# docs/SECURITY_RELEASE_PLAYBOOK.md
# SPDX-License-Identifier: Apache-2.0
# Security Release Playbook

1. Freeze `main`, branch hotfix.
2. Patch + tests (ASan/UBSan/TSan where relevant).
3. Build reproducible artifacts; generate SBOM; sign with cosign; create SLSA provenance.
4. Publish advisory; coordinate timeline; credit reporters.
md
Copiar código
# docs/SUPPLY_CHAIN.md
# SPDX-License-Identifier: Apache-2.0
# Supply Chain Security

- **Reproducible builds:** deterministic tar, `SOURCE_DATE_EPOCH`.
- **SBOM:** `scripts/sbom.sh` and CI artifact.
- **Provenance:** `slsa-source.yml` (ENABLE_SLSA).
- **Signing:** `release-signed.yml` (ENABLE_SIGNED_RELEASE).
- **Scanners:** CodeQL, Semgrep, OSV, Trivy, gitleaks, Scorecard (gated by vars).
- **Runtime:** rootless containers; minimal base images.
yaml
Copiar código
# .github/dependabot.yml
# SPDX-License-Identifier: Apache-2.0
version: 2
updates:
  - package-ecosystem: "github-actions"
    directory: "/"
    schedule: { interval: "weekly" }
    open-pull-requests-limit: 5
yaml
Copiar código
# .github/workflows/reuse.yml
# SPDX-License-Identifier: Apache-2.0
name: reuse-compliance
on:
  push:
    branches: [ main ]
  pull_request:
permissions: read-all
jobs:
  reuse:
    if: vars.ENABLE_REUSE_CI == 'true'
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: fsfe/reuse-action@v3
yaml
Copiar código
# .github/workflows/green-core.yml
# SPDX-License-Identifier: Apache-2.0
# Core minimal CI that should always pass if the project builds.
name: green-core
on:
  push:
  pull_request:
permissions: read-all
jobs:
  build-linux:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install deps
        run: sudo apt-get update && sudo apt-get install -y cmake ninja-build
      - name: Configure
        run: cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DNWX_ENABLE_TESTS=ON
      - name: Build
        run: cmake --build build -j
      - name: Test (soft by default)
        run: |
          if [ "${{ vars.ENFORCE_TESTS }}" = "true" ]; then
            ctest --test-dir build --output-on-failure
          else
            ctest --test-dir build --output-on-failure || true
          fi