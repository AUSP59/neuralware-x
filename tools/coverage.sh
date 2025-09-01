# SPDX-License-Identifier: Apache-2.0
#!/usr/bin/env bash
set -euo pipefail
./scripts/build.sh -DCMAKE_BUILD_TYPE=Debug -DNWX_ENABLE_COVERAGE=ON
./scripts/test.sh
command -v gcovr >/dev/null && gcovr -r . --exclude tests --xml coverage.xml --html-details coverage.html || echo "Install gcovr for reports"
