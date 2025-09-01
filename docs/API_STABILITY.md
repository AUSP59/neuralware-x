<!-- SPDX-License-Identifier: Apache-2.0 -->
# API Stability Policy

- Public headers under `include/` follow Semantic Versioning.
- **Minor** releases may add new APIs but do not remove or change existing signatures (source/backwards compatible).
- **Major** releases may remove or change APIs; deprecations announced at least one minor in advance.
