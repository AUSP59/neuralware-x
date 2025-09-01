// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "nwx/quant.hpp"
#include "nwx/model.hpp"

namespace nwx {
inline void enable_runtime_quantization(MLP& m){
  m.quant.l1 = quantize_weights(m.W1);
  m.quant.l2 = quantize_weights(m.W2);
  m.quant.enabled = true;
}
} // namespace nwx
