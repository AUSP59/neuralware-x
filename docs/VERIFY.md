
# How to Verify Releases and Integrity

1. **Checksums**
   ```bash
   sha256sum -c SHA256SUMS.txt
   shasum -a 512 -c SHA512SUMS.txt
   ```

2. **Cosign signature (optional)**
   ```bash
   cosign verify-blob --yes --signature nwx.sig nwx-<ver>-linux-x86_64.tar.gz
   ```

3. **SBOM**
   Open `sbom.cdx.json` and review packages. Compare two SBOMs using `scripts/sbom_diff.sh`.

4. **Repo inventory**
   See `docs/INVENTORY.md` (SHA256 per file).
