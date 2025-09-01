<!-- SPDX-License-Identifier: Apache-2.0 -->
# Calibration (Temperature Scaling)

Enable calibration after training using a validation split:
```bash
./out/bin/nwx --dataset data.csv --epochs 1000 --val_split 0.2 --calibrate_temp
```
The CLI performs a grid-search over T ∈ [0.5, 5.0] minimizing validation **NLL** and stores `temperature` in the model bundle. Inference divides logits by T before softmax.
