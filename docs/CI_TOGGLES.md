# CI Toggles (avoid red X by default)

The repository defines optional workflows that are **disabled by default** to keep initial pushes green.
Enable them by defining repository **Variables** (Settings → Secrets and variables → Variables) with value `true`:

- `ENABLE_DOCS` — Build & deploy docs to GitHub Pages
- `ENABLE_SECURITY_SCANS` — OSV/Trivy/SBOM jobs
- `ENABLE_FUZZ` — Fuzzing cron job
- `ENABLE_SEMGREP` — Semgrep SAST
- `ENABLE_HADOLINT` — Dockerfile lint
- `ENFORCE_COMMITLINT` — Enforce Conventional Commits
- `ENABLE_LINK_CHECK` — Lychee link checker
- `ENABLE_SCANBUILD` — Clang Static Analyzer
- `ENABLE_SANITIZERS_CI` — ASan/UBSan/TSan jobs
- `ENFORCE_COVERAGE` — Coverage gate threshold
- `ENFORCE_FORMAT` — clang-format gate
- `ENFORCE_BINARY_ARTIFACTS` — Block large/binary artifacts in repo
- `ENABLE_PGO_CI` — PGO two-phase build
- `ENABLE_CONTAINER_PUBLISH` — Push images to GHCR
- `ENABLE_FULL_MATRIX` — Build macOS and Windows in `ci-build.yml`

Start **only** with `ci-build.yml` and `ci-quality.yml`. Then enable toggles progressively.


Additional toggles:
- `ENABLE_CODEQL` — Enable CodeQL SAST
- `ENABLE_SCORECARD` — Enable OpenSSF Scorecard job
- `ENABLE_REUSE_CI` — Enable REUSE compliance CI
- `ENABLE_LICENSE_SCAN` — Enable license scan with licensee
- `ENABLE_RELEASE_PLEASE` — Enable automated release PRs


Additional toggles:
- `ENABLE_SLSA` — Enable SLSA provenance generator
- `ENABLE_SIGNED_RELEASE` — Enable signed releases workflow on tags
