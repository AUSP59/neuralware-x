# Contributing to NEURALWARE-X

Thanks for helping make NEURALWARE-X outstanding. This document explains **how to propose changes** safely and efficiently. Please read it fully before opening an Issue or Pull Request (PR).

> TL;DR  
> - Be kind: follow the **Code of Conduct**.  
> - Small, focused PRs with tests.  
> - Use **Conventional Commits** + **DCO sign-off**.  
> - Keep **clang-format**/**clang-tidy** happy and docs updated.  
> - Security issues: **do not** open public issues—use private reporting.

---

## 1) Ground rules

- **Code of Conduct**: See `CODE_OF_CONDUCT.md`. We enforce it.
- **License**: Apache-2.0. By contributing you agree your changes are licensed under Apache-2.0.
- **DCO Sign-off required**: Add a `Signed-off-by:` trailer to each commit. Example:

  ```text
  Signed-off-by: Your Name <you@example.com>
For you (repo owner), this would be:

text
Copiar código
Signed-off-by: AUSP59 <alanursapu@gmail.com>
Use git commit -s to add it automatically.

Security: Never include secrets or test credentials. See .gitleaks.toml.
Vulnerabilities must be reported privately (see Security below).

2) Development setup
Option A — Devcontainer (VS Code)
Open the repository in VS Code.

Reopen in Container.

Build & test:

bash
Copiar código
./scripts/build.sh
./scripts/test.sh
Option B — Nix shell
bash
Copiar código
nix develop          # or: nix-shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DNWX_ENABLE_TESTS=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
Option C — Local toolchain
Install: CMake ≥3.20, Ninja, Clang/GCC, Python3 (Sphinx), Doxygen, graphviz.

bash
Copiar código
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DNWX_ENABLE_TESTS=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
3) Branching & workflow
Main is protected. Branch from main: feature/<topic>, fix/<bug>, docs/<area>.

Keep PRs small & focused. One logical change per PR.

Rebase over main before final review.

4) Commits & PR titles
We use Conventional Commits:

vbnet
Copiar código
feat(parser): add streaming mode
fix(io): handle EINTR on Linux
docs: add profiling guide
perf(core): reduce allocations in hot path
refactor: extract decoder interface
test: add fuzz seeds for tokenizer
build: bump cmake minimum to 3.24
ci: enable clang-tidy on PRs
Include DCO sign-off on each commit (git commit -s).

Keep subject ≤ 72 chars; add details in the body when useful.

5) Style, linting & quality
Format: .clang-format (LLVM base, column limit 100). Run:

bash
Copiar código
clang-format -i $(git ls-files '*.cc' '*.cpp' '*.cxx' '*.h' '*.hpp' '*.hxx')
Lint: .clang-tidy rules. Run locally via your IDE or:

bash
Copiar código
cmake -S . -B build-tidy -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
run-clang-tidy -p build-tidy
Pre-commit (fast checks):

bash
Copiar código
pre-commit run --all-files || true
Static analysis (opt-in CI): scan-build, CodeQL, Semgrep.

6) Tests & coverage
Add or update unit/integration tests for every code change.

Run locally:

bash
Copiar código
ctest --test-dir build --output-on-failure
Coverage (local):

bash
Copiar código
cmake -S . -B build-cov -DNWX_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-cov -j
ctest --test-dir build-cov --output-on-failure
gcovr -r . --branches --html --html-details -o coverage.html
CI is configured to be green by default: tests are soft-fail in green-core unless you set ENFORCE_TESTS=true in repo Variables. See CI toggles below.

7) API stability & documentation
Public API lives under include/. Internal details in namespace detail.

Changes to public headers:

Document in CHANGELOG.md.

Use [[deprecated("since X.Y")]] before removal (keep for one MINOR release).

Docs:

Doxygen comments for public headers.

Sphinx site (docs/sphinx) for guides. Build locally:

bash
Copiar código
doxygen docs/doxygen/Doxyfile
pip install -r docs/sphinx/requirements.txt
sphinx-build -b html docs/sphinx docs_site
8) Performance
Respect Performance Budgets (docs/PERFORMANCE_BUDGETS.md).

Profiling:

bash
Copiar código
# PGO two-phase build using tests as workload
./scripts/pgo_build.sh

# Optional BOLT post-link optimization (requires llvm-bolt + perf data)
./scripts/bolt_optimize.sh build/bin/nwx perf.fdata
Include benchmark evidence for performance-sensitive PRs.

9) Dependencies & licensing
Avoid new runtime dependencies unless necessary. If needed:

Discuss in an Issue/RFC first.

Add to vcpkg.json/conanfile.py (and package templates if relevant).

Ensure license compatibility with Apache-2.0.

Update 3RD-PARTY-LICENSES.md if vendoring (prefer not to vendor).

Every source file must include an SPDX header (already enforced).
REUSE compliance is configured; keep it passing when enabled.

10) Security
Do NOT report vulnerabilities in public issues or PRs.
Use GitHub Private Vulnerability Reporting or email alanursapu@gmail.com.
See .github/SECURITY.md, docs/INCIDENT_RESPONSE.md, docs/SECURITY_RELEASE_PLAYBOOK.md, docs/THREAT_MODEL.md.

Never commit secrets. gitleaks and GitHub secret scanning will complain (by design).

Containers must stay rootless. Keep base images minimal.

11) CI toggles (to avoid red ✗ until you’re ready)
Set repository Variables (Settings → Secrets and variables → Variables) to true to enable:

Strictness: ENFORCE_TESTS, ENFORCE_FORMAT, ENFORCE_COVERAGE, ENFORCE_BINARY_ARTIFACTS

Quality & Security: ENABLE_COVERAGE_JOB, ENABLE_SCANBUILD, ENABLE_SEMGREP, ENABLE_CODEQL, ENABLE_REUSE_CI, ENABLE_LICENSE_SCAN, ENABLE_SCORECARD

Builds & Features: ENABLE_DOCS, ENABLE_SANITIZERS_CI, ENABLE_PGO_CI, ENABLE_CONTAINER_PUBLISH, ENABLE_FULL_MATRIX

Releases & Provenance: ENABLE_RELEASE, ENABLE_SIGNED_RELEASE, ENABLE_SLSA, ENABLE_RELEASE_PLEASE

See docs/CI_TOGGLES.md and docs/GREEN_CHECKS_README.md.

12) Packaging & releases
Packaging templates: packaging/ (Homebrew, Debian, winget), vcpkg.json, conanfile.py.

Releases:

Regular CI: release.yml (opt-in via ENABLE_RELEASE).

Signed releases: release-signed.yml (enable ENABLE_SIGNED_RELEASE).

SLSA provenance: slsa-source.yml (enable ENABLE_SLSA).

SBOM is attached to releases; verify via docs/VERIFY.md.

13) Accessibility & inclusion
Write inclusive docs/messages (docs/INCLUSIVE_LANGUAGE.md).

Keep accessibility goals in mind (ACCESSIBILITY.md, docs/VPAT.md).

14) Opening Issues & PRs
Issues: describe the problem, expected behavior, repro steps, environment.

Feature requests: describe the problem first, not the solution; propose minimal viable API.

PR checklist:

 Small, focused commits with DCO sign-off

 Tests updated/added

 Docs updated (API/README/CHANGELOG as needed)

 clang-format applied

 CI green locally or acceptable (if strict gates disabled)

15) Governance & escalation
Maintainers and decision making are defined in GOVERNANCE.md, MAINTAINERS.md, and docs/RACI.md.

For mediator/escalation (process concerns), ping @AUSP59.

16) Contact
GitHub: @AUSP59

Security (private): alanursapu@gmail.com

Press/brand: docs/PRESS_KIT.md, docs/BRAND_GUIDE.md

Thank you for contributing!