# SPDX-License-Identifier: Apache-2.0
#!/usr/bin/env bash
set -euo pipefail
mkdir -p out
cmake -S . -B out "$@"
cmake --build out --parallel
