// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <vector>
#include <cstdint>
#include "nwx/tensor.hpp"
#include "nwx/model.hpp"

namespace nwx {

struct QuantLayer {
  // Per-output-channel weight scales and int8 weights in row-major (out x in)
  std::vector<int8_t> Wq;
  std::vector<double> sw; // scale per output neuron
  int out{0}, in{0};
};

struct QuantMLP {
  bool enabled{false};
  QuantLayer l1, l2;
};

// Quantize float weights to int8 per-output-channel
inline QuantLayer quantize_weights(const Tensor2D& W) {
  QuantLayer q; q.out = W.rows; q.in = W.cols;
  q.Wq.resize((size_t)W.rows * W.cols);
  q.sw.resize(W.rows);
  for (int o=0;o<W.rows;++o){
    double maxabs = 1e-12;
    for (int i=0;i<W.cols;++i){ double v = std::abs(W(o,i)); if (v>maxabs) maxabs=v; }
    double s = maxabs / 127.0;
    if (s < 1e-12) s = 1.0/127.0;
    q.sw[o] = s;
    for (int i=0;i<W.cols;++i){
      int idx = o*W.cols + i;
      int v = (int)std::lround(W(o,i) / s);
      if (v>127) v=127; if (v<-127) v=-127;
      q.Wq[(size_t)idx] = (int8_t)v;
    }
  }
  return q;
}

// Dynamic activation scale from a batch: symmetric max
inline double dynamic_act_scale(const Tensor2D& X){
  double m = 1e-12;
  for (double v : X.data){ double a = std::abs(v); if (a>m) m=a; }
  return m / 127.0;
}

// INT8 GEMM: (rows x k) * (out x k)^T => (rows x out). Wq is row-major (out x k).
// Accumulate in int32, then dequantize by (sx * sw[out]).
Tensor2D gemm_i8_dequant(const Tensor2D& X, const QuantLayer& ql, double sx);

} // namespace nwx
