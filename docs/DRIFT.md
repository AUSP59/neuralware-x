<!-- SPDX-License-Identifier: Apache-2.0 -->
# Data Drift (PSI)

Compare baseline vs current datasets:
```bash
./out/bin/nwx_drift --baseline data_baseline.csv --current data_today.csv --bins 10
```
Outputs Population Stability Index per feature.
