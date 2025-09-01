# SPDX-License-Identifier: Apache-2.0
#!/usr/bin/env bash
set -euo pipefail
ctest --test-dir out --output-on-failure || (echo "Tests failed"; exit 1)
