<!-- SPDX-License-Identifier: Apache-2.0 -->
# Explanations (gradient*input)

`nwx_explain` prints top-k feature contributions per row:
```
./out/bin/nwx_explain --dataset data.csv --model model.bundle --topk 5
```
This is a fast, approximate attribution method.
