# SPDX-License-Identifier: Apache-2.0
#!/usr/bin/env bash
set -euo pipefail
echo "Build"
./scripts/build.sh -DCMAKE_BUILD_TYPE=Release
echo "Test"
./scripts/test.sh
echo "OK"
