# SPDX-License-Identifier: Apache-2.0
#!/usr/bin/env bash
set -euo pipefail
echo "clang-tidy (if available)"
command -v clang-tidy >/dev/null || { echo "clang-tidy not found, skipping"; exit 0; }
SRC=$(git ls-files | grep -E '\.(cpp|hpp|h)$' || true)
[ -z "$SRC" ] && exit 0
clang-tidy $SRC -- -Iinclude -std=c++20
