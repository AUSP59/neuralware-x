<!-- SPDX-License-Identifier: Apache-2.0 -->
# Training Tips

- Try `--optimizer adamw --wd 0.01 --lr_schedule cosine --warmup_steps 20 --clip_norm 1.0`.
- Use `--standardize` for tabular data; the scaler is stored in the bundle.
