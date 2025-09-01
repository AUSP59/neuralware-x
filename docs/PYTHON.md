<!-- SPDX-License-Identifier: Apache-2.0 -->
# Python Bindings (ctypes)

Build shared lib and install package:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target nwx_capi --parallel
pip install -e python/
python - << 'PY'
import numpy as np; from neuralwarex import NeuralWareX
m = NeuralWareX('model.bundle'); print(m.predict(np.array([[0,1],[1,0]], float)))
PY
```
