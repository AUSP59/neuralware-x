
#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
if [ $# -ne 2 ]; then
  echo "Usage: $0 <old-sbom.json> <new-sbom.json>" >&2
  exit 1
fi
old="$1"; new="$2"
jq -S '.' "$old" > /tmp/old_s.json
jq -S '.' "$new" > /tmp/new_s.json
diff -u /tmp/old_s.json /tmp/new_s.json || true
