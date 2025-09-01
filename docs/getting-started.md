<!-- SPDX-License-Identifier: Apache-2.0 -->
# Getting Started

```bash
./scripts/build.sh -DCMAKE_BUILD_TYPE=Release
./out/bin/nwx --dataset ./examples/xor.csv --epochs 800 --optimizer adam --standardize --save model.bundle
./out/bin/nwx_serve model.bundle 8080 --token SECRET
```
