<!-- SPDX-License-Identifier: Apache-2.0 -->
# SSE Streaming

```
curl -N -H 'Content-Type: application/json' -d '{"instances":[[0,1],[1,0]]}' http://localhost:8080/predict/stream
```
One SSE event per row: `data: {"row":0,"probs":[...]}`
