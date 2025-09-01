
#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
target="${1:-install}"
out256="${2:-SHA256SUMS.txt}"
out512="${3:-SHA512SUMS.txt}"
find "$target" -type f -print0 | sort -z | xargs -0 sha256sum > "$out256"
find "$target" -type f -print0 | sort -z | xargs -0 shasum -a 512 > "$out512"
echo "Wrote $out256 and $out512"
