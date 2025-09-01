<!-- SPDX-License-Identifier: Apache-2.0 -->
# Evaluation CLI

Use `nwx_eval` to compute metrics from a dataset and either a model bundle or a CSV of probabilities.

```bash
./out/bin/nwx_eval --dataset ./examples/xor.csv --model model.bundle
# or with precomputed probabilities:
./out/bin/nwx_eval --dataset ./examples/xor.csv --probs probs.csv
```
Outputs: accuracy, macro/micro F1, ECE, ROC-AUC (binary or OvR).
