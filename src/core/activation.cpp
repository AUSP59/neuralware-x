// SPDX-License-Identifier: Apache-2.0
#include "nwx/activation.hpp"
#include <cmath>

namespace nwx {

static Tensor2D tanh_act(const Tensor2D& X) {
  Tensor2D Y = X;
  for (int i=0;i<X.rows;++i) for (int j=0;j<X.cols;++j) Y(i,j) = std::tanh(X(i,j));
  return Y;
}
static Tensor2D tanh_grad_act(const Tensor2D& X) {
  Tensor2D G(X.rows, X.cols);
  for (int i=0;i<X.rows;++i) for (int j=0;j<X.cols;++j) { double t = std::tanh(X(i,j)); G(i,j) = 1.0 - t*t; }
  return G;
}

Tensor2D activate(const Tensor2D& X, Activation a) {
  switch (a) {
    case Activation::Relu: return relu(X);
    case Activation::Tanh: return tanh_act(X);
  }
  return X;
}
Tensor2D activate_grad(const Tensor2D& pre, Activation a) {
  switch (a) {
    case Activation::Relu: return relu_grad(pre);
    case Activation::Tanh: return tanh_grad_act(pre);
  }
  return Tensor2D(pre.rows, pre.cols);
}

} // namespace nwx
