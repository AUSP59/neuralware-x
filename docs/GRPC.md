<!-- SPDX-License-Identifier: Apache-2.0 -->
# gRPC Server (optional)

- Proto: `proto/nwx.proto`
- Build: `cmake --preset grpc` (requires installed gRPC + Protobuf)
- Run: `./out/bin/nwx_grpc_serve model.bundle 9090`


## Streaming & Batching
- `PredictStream`: bidirectional stream of rows ↔ probabilities.
- `BatchPredict`: identical to `Predict`, provided for explicit batch semantics from some SDKs.
