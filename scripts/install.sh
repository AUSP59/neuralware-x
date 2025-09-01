# SPDX-License-Identifier: Apache-2.0
#!/usr/bin/env bash
set -euo pipefail
prefix=/usr/local
while [[ $# -gt 0 ]]; do
  case "$1" in
    --prefix) prefix="$2"; shift 2;;
    *) echo "Unknown arg: $1"; exit 2;;
  esac
done
cmake --install out --prefix "$prefix"
echo "Installed to $prefix"
