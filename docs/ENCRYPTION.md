<!-- SPDX-License-Identifier: Apache-2.0 -->
# Encrypted Bundles (AES-256-GCM)

Encrypt and serve:
```bash
./out/bin/nwx_encrypt --in model.bundle --out model.bundle.enc --encrypt --key <64-hex>
export NWX_BUNDLE_KEY=<64-hex>
./out/bin/nwx_serve model.bundle.enc 8080 --token SECRET
```
