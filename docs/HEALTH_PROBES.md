<!-- SPDX-License-Identifier: Apache-2.0 -->
# Health & Readiness

- `GET /healthz` => 200 `{"status":"ok"}`
- `GET /ready`   => 200/503 depending on whether the model bundle was loaded.
