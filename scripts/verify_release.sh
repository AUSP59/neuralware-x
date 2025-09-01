
#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
artifact="${1:-nwx.tar.gz}"
sha_file="${2:-SHA256SUMS.txt}"
sig="${3:-nwx.sig}"
echo "[*] Verifying checksums..."
sha256sum -c "${sha_file}" || shasum -a 256 -c "${sha_file}"
echo "[*] (Optional) Verifying cosign signature..."
if command -v cosign >/dev/null 2>&1; then
  COSIGN_EXPERIMENTAL=1 cosign verify-blob --yes --signature "${sig}" "${artifact}" || true
else
  echo "cosign not installed; skipping signature verification"
fi
