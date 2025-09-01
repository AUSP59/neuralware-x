#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
# Generates a CycloneDX SBOM for the source tree using `syft` (if installed).
# Usage: scripts/sbom.sh [OUTPUT_JSON]
out="${1:-bom/sbom.cdx.json}"
mkdir -p "$(dirname "$out")"
if ! command -v syft >/dev/null 2>&1; then
  echo "syft not installed. Install from https://github.com/anchore/syft" >&2
  exit 2
fi
syft packages dir:. -o cyclonedx-json > "$out"
echo "SBOM written to $out"
