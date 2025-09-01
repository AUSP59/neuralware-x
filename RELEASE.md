<!-- SPDX-License-Identifier: Apache-2.0 -->
# Release Process

1. Ensure CI is green. Run `./tools/verify.sh` locally.
2. Bump version in `src/core/version.hpp` and `CMakeLists.txt`.
3. Update `CHANGELOG.md`.
4. Tag `vX.Y.Z` and push.
5. CI builds artifacts and creates a GitHub Release with CPack packages.
