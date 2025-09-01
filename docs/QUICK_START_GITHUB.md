# Quick Start for GitHub (Green Checks)

This repository is configured to pass CI by default on first push.
Most "heavy" workflows are disabled via repository **Variables** and can be enabled progressively.

## Steps
1. Create a new GitHub repo (empty), then push this repository's contents.
2. Check Actions: you should see only Linux build and basic quality jobs running and passing.
3. Enable optional workflows one-by-one by setting Variables (Settings → Secrets and variables → **Variables**) to `true`:
   - `ENFORCE_FORMAT` (recommended first)
   - `ENABLE_DOCS` (then enable GitHub Pages)
   - `ENABLE_SECURITY_SCANS`
   - `ENABLE_CODEQL`, `ENABLE_SCORECARD`
   - `ENFORCE_COVERAGE` (adjust threshold in `coverage-gate.yml` if needed)
   - Containers, PGO, provenance/signing, etc.

4. For signed releases / provenance:
   - Set `ENABLE_SIGNED_RELEASE=true` and `ENABLE_SLSA=true`, then tag `vX.Y.Z`.
   - Ensure org allows OIDC and code signing (cosign).

5. To expand OS coverage:
   - Set `ENABLE_FULL_MATRIX=true` in repo Variables to build macOS and Windows.

## Troubleshooting
- Red X on docs deploy: enable GitHub Pages and set `ENABLE_DOCS=true`.
- Red X on container publish: set `ENABLE_CONTAINER_PUBLISH=true` and ensure GHCR permissions.
- Coverage gate failing: lower threshold temporarily or increase tests.
- Conventional commits failing: disable (`ENFORCE_COMMITLINT=false`) or fix PR title.

