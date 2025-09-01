#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
if ! command -v checksec >/dev/null 2>&1; then
  echo "checksec not found; skipping"; exit 0
fi
bin="${1:-build/bin/nwx_serve}"
if [ ! -x "$bin" ]; then echo "Binary $bin not found"; exit 0; fi
checksec --file="$bin" || true
