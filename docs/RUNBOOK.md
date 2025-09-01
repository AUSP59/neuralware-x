<!-- SPDX-License-Identifier: Apache-2.0 -->
# Runbook
- Start: `nwx_serve model.bundle 8080 --token SECRET`
- Reload: POST `/reload` or send `SIGHUP`
- Health: `/healthz`, `/readyz`, `/metrics`
- JWT claims: set `NWX_JWT_ISS`, `NWX_JWT_AUD`, optional `NWX_JWT_LEEWAY_S`
