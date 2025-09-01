# Green Checks Guide

This repository ships with most strict workflows disabled by repository Variables to ensure green checks on first push.
Toggle them in Settings → Secrets and variables → **Variables**.

## Strictness Toggles (set to `true` to enable)
- ENFORCE_TESTS — Fail CI if `ctest` fails (default: soft-fail).
- ENABLE_COVERAGE_JOB — Run the coverage job in `ci-quality.yml`.
- ENFORCE_FORMAT — Enforce clang-format gate.
- ENFORCE_BINARY_ARTIFACTS — Block large/ELF/PE binaries in repo.
- ENFORCE_COVERAGE — Enforce coverage threshold (see `coverage-gate.yml`).

## Features Toggles
- ENABLE_DOCS — Build & deploy docs (GitHub Pages).
- ENABLE_SECURITY_SCANS — OSV/Trivy/SBOM.
- ENABLE_SEMGREP — Semgrep SAST.
- ENABLE_HADOLINT — Dockerfile lint.
- ENABLE_LINK_CHECK — Link checker.
- ENABLE_SCANBUILD — clang static analyzer.
- ENABLE_SANITIZERS_CI — ASan/UBSan/TSan.
- ENABLE_PGO_CI — PGO two-phase build.
- ENABLE_CONTAINER_PUBLISH — GHCR images.
- ENABLE_FULL_MATRIX — macOS/Windows in CI.
- ENABLE_CODEQL — CodeQL scan.
- ENABLE_SCORECARD — OpenSSF Scorecard.
- ENABLE_REUSE_CI — REUSE compliance.
- ENABLE_LICENSE_SCAN — License scan (licensee).
- ENABLE_RELEASE_PLEASE — Automated release PRs.
- ENABLE_SLSA — SLSA provenance.
- ENABLE_SIGNED_RELEASE — Signed release.
- ENABLE_RELEASE — Release workflow on tags.
