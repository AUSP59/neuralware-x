<!-- SPDX-License-Identifier: Apache-2.0 -->
# Public API

```c++
#include "nwx/model.hpp"
#include "nwx/trainer.hpp"

nwx::MLP m;
std::mt19937 rng(7);
m.init(/*in=*/2, /*hidden=*/4, /*out=*/2, rng);

nwx::Dataset ds = /* ...load... */;
nwx::TrainConfig cfg; cfg.epochs=2000; cfg.lr=0.1;

nwx::train_xent(m, ds, cfg);
```
