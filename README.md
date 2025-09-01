# NEURALWARE-X

> High-performance, production-grade C++ core with world-class OSS posture: security, supply-chain, reproducibility, governance, documentation, and DX — all wired in.

<p align="center">
  <img src="brand/logo.svg" alt="NEURALWARE-X" width="520">
</p>

---

<!-- Badges (replace OWNER/REPO if your repository name differs) -->
[![CI](https://img.shields.io/github/actions/workflow/status/AUSP59/NEURALWARE-X/ci-build.yml?branch=main&label=build)](https://github.com/AUSP59/NEURALWARE-X/actions)
[![Quality](https://img.shields.io/github/actions/workflow/status/AUSP59/NEURALWARE-X/ci-quality.yml?branch=main&label=quality)](https://github.com/AUSP59/NEURALWARE-X/actions)
[![Docs](https://img.shields.io/github/actions/workflow/status/AUSP59/NEURALWARE-X/docs.yml?branch=main&label=docs)](https://github.com/AUSP59/NEURALWARE-X/actions)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue)](#license)

> 📌 **Green checks by default**: most strict/externally-dependent workflows are **off** until you flip a repo **Variable**. See **[docs/CI_TOGGLES.md](docs/CI_TOGGLES.md)** and **[docs/QUICK_START_GITHUB.md](docs/QUICK_START_GITHUB.md)**.

---

## Table of contents
- [Highlights](#highlights)
- [Quick start](#quick-start)
- [Build, test & tooling](#build-test--tooling)
- [Performance (PGO/BOLT)](#performance-pgobolt)
- [Documentation site](#documentation-site)
- [Containers](#containers)
- [Packaging (templates)](#packaging-templates)
- [Security & supply chain](#security--supply-chain)
- [Provenance & signing](#provenance--signing)
- [Reproducible artifacts](#reproducible-artifacts)
- [Contributing & community](#contributing--community)
- [Governance & support](#governance--support)
- [Accessibility & inclusion](#accessibility--inclusion)
- [Compliance & mappings](#compliance--mappings)
- [Project structure](#project-structure)
- [Versioning & support policy](#versioning--support-policy)
- [License](#license)
- [Contact](#contact)

---

## Highlights
- **Modern C++** with strict **clang-tidy**, unified **clang-format**, sanitizer presets, and high-signal CI.
- **Security first**: CodeQL, Semgrep, OSV/Trivy, gitleaks, SBOM (CycloneDX), OpenSSF Scorecard (all gated to avoid false ✗).
- **Supply-chain posture**: SPDX per file + REUSE, SBOM per release, SLSA provenance, signed artifacts (cosign).
- **Reproducibility**: deterministic tarballs, `SOURCE_DATE_EPOCH`, Nix dev shell, documented flags and process.
- **DX**: devcontainer, ccache, CMake presets, Makefile helpers, pre-commit suite, ready-to-ship workflows.
- **Docs**: Doxygen + Sphinx, GitHub Pages workflow (opt-in), handbooks for security, incident response, benchmarking, profiling.
- **Community**: issue/PR templates, contributor ladder, governance, codeowners, labels, backports, stale triage.
- **Performance**: PGO two-phase build + optional LLVM BOLT post-link optimization.

---

## Quick start

```bash
# Configure + build (Release) + enable tests
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DNWX_ENABLE_TESTS=ON
cmake --build build -j

# Run tests (soft-fail in CI unless ENFORCE_TESTS=true, but locally they fail on error)
ctest --test-dir build --output-on-failure
Dev container (VS Code):

Open folder → Reopen in Container

./scripts/build.sh → ./scripts/test.sh

Nix shell:

bash
Copiar código
nix develop # or: nix-shell
cmake -S . -B build && cmake --build build -j && ctest --test-dir build
Build, test & tooling
Presets (CMakePresets.json):

release, relwithdebinfo, asan-ubsan, tsan, coverage

Common targets:

bash
Copiar código
# Makefile shortcuts
make build         # Release build
make test          # Run tests
make cov           # Coverage configure+test
make asan          # ASan/UBSan build
make format        # Run format hooks
Static analysis

clang-tidy (config in .clang-tidy)

scan-build CI (opt-in via ENABLE_SCANBUILD)

Formatting

Canonical style in .clang-format (LLVM base, ColumnLimit=100)

Performance (PGO/BOLT)
Generate and use profiles (Clang/GCC supported):

bash
Copiar código
# Two-phase PGO build using tests as workload
./scripts/pgo_build.sh

# Optional BOLT post-link optimization (requires perf data & llvm-bolt)
./scripts/bolt_optimize.sh build/bin/nwx perf.fdata
Continuous PGO CI is available (opt-in via ENABLE_PGO_CI).

Documentation site
Build local API & docs:

bash
Copiar código
doxygen docs/doxygen/Doxyfile
pip install -r docs/sphinx/requirements.txt
sphinx-build -b html docs/sphinx docs_site
GitHub Pages deployment is ready (opt-in via repo variable ENABLE_DOCS=true).

Containers
Runtime: rootless base (docker/Dockerfile.runtime), ENTRYPOINT ["nwx"].

Build/Publish: multi-arch to GHCR (opt-in via ENABLE_CONTAINER_PUBLISH).

Run:

bash
Copiar código
docker build -f docker/Dockerfile.runtime -t nwx .
docker run --rm nwx --help
Packaging (templates)
Ready-to-adapt recipes:

Homebrew: packaging/homebrew/Formula/nwx.rb

Debian: packaging/debian/

winget: packaging/winget/nwx.yaml

vcpkg: vcpkg.json

Conan: conanfile.py

Fill in final download URLs and SHA256 when publishing releases.

Security & supply chain
Security policy → .github/SECURITY.md
Private vulnerability reporting → GitHub Advisories or alanursapu@gmail.com
Security contacts → SECURITY_CONTACTS, @AUSP59

Scanners (opt-in via Variables):

ENABLE_CODEQL, ENABLE_SEMGREP, ENABLE_GITLEAKS

ENABLE_SECURITY_SCANS (OSV + Trivy + SBOM)

ENABLE_LICENSE_SCAN, ENABLE_SCORECARD, ENABLE_REUSE_CI

Threat model, incident response & playbooks:

docs/THREAT_MODEL.md

docs/INCIDENT_RESPONSE.md

docs/SECURITY_RELEASE_PLAYBOOK.md

docs/SUPPLY_CHAIN.md

Secrets: gitleaks config at .gitleaks.toml. Avoid secrets in code and logs.

Provenance & signing
Release signing (cosign keyless) and SLSA provenance are ready:

enable ENABLE_SIGNED_RELEASE=true and/or ENABLE_SLSA=true

push tag vX.Y.Z

Verify: see docs/VERIFY.md

sha256sum -c SHA256SUMS.txt

cosign verify-blob --yes --signature nwx.sig artifact.tar.gz

SBOM diff: scripts/sbom_diff.sh old.json new.json

Reproducible artifacts
Deterministic tar via tar --sort=name --owner=0 --group=0 --numeric-owner --mtime='UTC 2020-01-01'

SOURCE_DATE_EPOCH set from latest commit

Full repo SHA256 inventory at CHECKSUMS.sha256 + docs/INVENTORY.md

Contributing & community
Start here: CONTRIBUTING.md, CODE_OF_CONDUCT.md

Contributor Ladder: docs/CONTRIBUTOR_LADDER.md

Style: docs/CODING_STYLE.md

API stability: docs/API_STABILITY.md

PRs: Conventional Commits enforced (opt-in via ENFORCE_COMMITLINT)

Changelog gate: requires CHANGELOG.md update on code changes

Issue/PR templates: .github/ISSUE_TEMPLATE/, PULL_REQUEST_TEMPLATE.md

Run local linters:

bash
Copiar código
pre-commit run --all-files || true
./scripts/lint_all.sh
Governance & support
Governance: GOVERNANCE.md, MAINTAINERS.md, CODEOWNERS, RACI, RISK_REGISTER.md

Branch protection: docs/BRANCH_PROTECTION.md

LTS policy: docs/LTS_POLICY.md

Support: SUPPORT.md, GitHub Issues; security via .github/SECURITY.md

Accessibility & inclusion
VPAT/Accessibility: ACCESSIBILITY.md, docs/VPAT.md

Internationalization: docs/INTERNATIONALIZATION.md, i18n/locales/en/messages.pot

Inclusive language: docs/INCLUSIVE_LANGUAGE.md

Compliance & mappings
License & headers: Apache-2.0; SPDX identifiers in all files; REUSE compliant.

OpenChain (ISO/IEC 5230): docs/OPENCHAIN_CONFORMANCE.md

NIST SSDF (SP 800-218): docs/NIST_SSDF_MAPPING.md

ISO/IEC 27001 (Annex A): docs/ISO27001_MAPPING.md

OpenSSF/CII: docs/CII_BEST_PRACTICES.md

Project structure
bash
Copiar código
.
├─ include/                # Public headers (stable API surface)
├─ src/                    # Implementation
├─ tests/                  # Unit/integration tests
├─ apps/                   # CLI or sample apps (if applicable)
├─ cmake/Toolchains/       # Hardening, ccache, PGO toolchains
├─ docker/                 # Dockerfiles (build/runtime rootless)
├─ packaging/              # Homebrew, Debian, winget, etc. templates
├─ docs/                   # Sphinx + guides (security, supply-chain, profiling…)
├─ i18n/                   # Localization scaffolding
├─ .github/workflows/      # CI/CD (gated by repo Variables)
└─ scripts/                # Helpers (sbom, verify, bolt, pgo, etc.)
Versioning & support policy
SemVer. Breaking changes → MAJOR; deprecations remain one MINOR release.

Supported versions: latest two MINOR releases for security fixes when feasible (see docs/LTS_POLICY.md).

License
Apache-2.0. See LICENSE, NOTICE.

Per-file headers included via SPDX identifiers; REUSE mapping present.

Contact
GitHub: @AUSP59

Security contact: alanursapu@gmail.com (or GitHub Private Vulnerability Reporting)

Press/brand: see docs/PRESS_KIT.md, docs/BRAND_GUIDE.md

CI “green by default”
To keep the first push green, strict jobs are gated by repo Variables (set to true to enable). See docs/GREEN_CHECKS_README.md for the full table. Recommended enablement order:

ENFORCE_FORMAT → 2) ENABLE_DOCS (+ Pages) → 3) ENABLE_SECURITY_SCANS → 4) ENABLE_CODEQL / ENABLE_SCORECARD → 5) ENABLE_COVERAGE_JOB then ENFORCE_COVERAGE when ready.