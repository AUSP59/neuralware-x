#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
command -v clang-format >/dev/null && clang-format -i $(git ls-files "*.c" "*.cc" "*.cpp" "*.cxx" "*.h" "*.hpp" "*.hxx") || echo "clang-format not installed"
