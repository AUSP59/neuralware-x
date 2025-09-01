<!-- SPDX-License-Identifier: Apache-2.0 -->
# /buildinfo Endpoint

Returns JSON with version, CPU features and whether TLS is enabled:
```
GET /buildinfo
{"version":"5.0.0","cpu":{"avx2":true,"fma":true},"tls":true}
```
