
# Release Checklist

- [ ] Update CHANGELOG and ROADMAP
- [ ] Tag `vX.Y.Z`
- [ ] Run `release-signed.yml` (artifacts + checksums + cosign)
- [ ] Attach SBOM and provenance (SLSA)
- [ ] Smoke test containers on amd64/arm64
