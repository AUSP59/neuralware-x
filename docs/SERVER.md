<!-- SPDX-License-Identifier: Apache-2.0 -->
# HTTP Prediction Server

Build and run:
```bash
./out/bin/nwx_serve model.bundle 8080 --token SECRET --rate 50 --burst 100
curl -H 'Authorization: Bearer SECRET' -H 'Content-Type: text/csv' -d '0,1\n1,0' http://localhost:8080/predict
curl http://localhost:8080/healthz
curl http://localhost:8080/readyz
curl http://localhost:8080/metrics
```
- **Auth**: Bearer token via `--token` or `NWX_TOKEN` environment variable.
- **Rate limiting**: token bucket (`--rate` RPS, `--burst` bucket size).
- **Metrics**: Prometheus counters + basic health endpoints.
