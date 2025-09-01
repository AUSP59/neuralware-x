#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
base="${1:-origin/main}"
changed=$(git diff --name-only "$base"...HEAD | grep -E '^(src/|include/|CMakeLists.txt|docs/)' || true)
if [ -n "$changed" ]; then
  if ! git diff --name-only "$base"...HEAD | grep -q '^CHANGELOG.md$'; then
    echo "Code/docs changed but CHANGELOG.md not updated. Please add an entry."
    exit 2
  fi
fi
echo "CHANGELOG gate OK"
