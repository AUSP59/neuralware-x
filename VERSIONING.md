<!-- SPDX-License-Identifier: Apache-2.0 -->
# Versioning Policy

We use **Semantic Versioning (SemVer)**: `MAJOR.MINOR.PATCH`.

- **MAJOR**: incompatible API changes.
- **MINOR**: backward-compatible functionality.
- **PATCH**: backward-compatible bug fixes.

## Stability and Compatibility

- Public C++ headers under `include/` define the **stable API**.
- Symbols not in headers (or marked `detail`) are internal and may change.
- We maintain **deprecation periods** of at least one MINOR before removal.
- Build system (`CMake` targets and exported config files) follows SemVer as well.

See `docs/API_STABILITY.md` for further details.
