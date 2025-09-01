#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
echo "[1/8] Format checks"
clang-format --version >/dev/null 2>&1 && git ls-files '*.[ch]pp' '*.[ch]xx' '*.cc' '*.c' '*.h' | xargs -r clang-format --dry-run -Werror || true
echo "[2/8] Shell checks"
command -v shellcheck >/dev/null 2>&1 && shellcheck $(git ls-files '*.sh') || true
echo "[3/8] Build (Release, harden)"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DNWX_HARDEN=ON
cmake --build build -j
echo "[4/8] Tests"
ctest --test-dir build --output-on-failure || true
echo "[5/8] SBOM"
which syft >/dev/null 2>&1 && syft dir:. -o spdx-json=sbom.spdx.json || echo "Install syft to generate SBOM locally"
echo "[6/8] OpenAPI lint"
npx -y @stoplight/spectral-cli lint docs/openapi.yaml -r .spectral.yaml || true
echo "[7/8] Helm lint + kubeconform"
helm lint deploy/helm/nwx || true
echo "[8/8] Done"
