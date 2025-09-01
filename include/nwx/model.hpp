// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tensor.hpp"
#include "activation.hpp"
#include <vector>
#include <random>

namespace nwx {

struct MLP {
  double temperature{1.0};
  QuantMLP quant;
  double temperature{1.0};
  Activation act{Activation::Relu};
  int in{0}, hidden{0}, out{0};
  Tensor2D W1, W2; // shapes: (in, hidden), (hidden, out)
  std::vector<double> b1, b2; // sizes: hidden, out

  Tensor2D x1, a1, a2; // caches for forward pass

  void init(int in_dim, int hidden_dim, int out_dim, std::mt19937& rng, Activation a = Activation::Relu);
  Tensor2D forward(const Tensor2D& X); // returns logits
  void backward(const Tensor2D& X, const std::vector<int>& y, double lr); // SGD step
};

} // namespace nwx
