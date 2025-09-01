<!-- SPDX-License-Identifier: Apache-2.0 -->
# Deterministic Builds

We follow best practices for reproducible builds:
- Source/date mapping flags (`-ffile-prefix-map`, `-fdebug-prefix-map`)
- Optional LTO/IPO via `-DNWX_ENABLE_LTO=ON` when supported
- Release artifacts produced from clean containers (CI)
- Documented environment and presets

Set `SOURCE_DATE_EPOCH` in CI to pin timestamps.
