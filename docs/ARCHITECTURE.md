<!-- SPDX-License-Identifier: Apache-2.0 -->
# Architecture

```
include/nwx/    # public headers
src/core/       # core math, model, trainer, json
src/io/         # csv and serialization
src/app/        # CLI entry point
tests/          # minitest + unit tests
scripts/        # build/test/install helpers
docs/           # whitepaper, architecture, threat model, etc.
```

- **Model**: 1‑hidden‑layer MLP (ReLU, Softmax CE).
- **Trainer**: full‑batch SGD for simplicity and determinism.
- **Dataset**: CSV numeric, last column is an integer class label.
- **Serialization**: portable binary for weights.
