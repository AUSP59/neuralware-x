<!-- SPDX-License-Identifier: Apache-2.0 -->
# Release Process

1. Create a release branch `release/vX.Y.Z`.
2. Update version, CHANGELOG, and docs.
3. Tag `vX.Y.Z` -> triggers `release-on-tag.yml` and `slsa-provenance.yml`.
4. Verify artifacts, SBOMs, signatures, and checksums.
5. Announce with links to docs and SBOM.
