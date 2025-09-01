
#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
BIN="${1:-build/bin/nwx}"
PROF="${2:-perf.fdata}"
if ! command -v llvm-bolt >/dev/null; then
  echo "llvm-bolt not installed"
  exit 0
fi
out="${BIN}.bolt"
llvm-bolt "$BIN" -o "$out" -data="$PROF" -reorder-blocks=cache+ -reorder-functions=hfsort+ -split-functions -split-all-cold -icf=1 -use-gnu-stack
echo "BOLT optimized binary at: $out"
