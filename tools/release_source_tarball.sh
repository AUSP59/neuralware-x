#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
export SOURCE_DATE_EPOCH=${SOURCE_DATE_EPOCH:-1714694400}
ver="${1:-$(git describe --tags --always --dirty || echo 0.0.0)}"
out="neuralware-x-${ver}.tar.gz"
git archive --format=tar --prefix="neuralware-x-${ver}/" HEAD | gzip -n > "$out"
echo "Created $out (deterministic gzip)"
