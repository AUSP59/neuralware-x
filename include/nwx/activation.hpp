// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tensor.hpp"
#include <string>

namespace nwx {

enum class Activation { Relu=0, Tanh=1 };

inline Activation parse_activation(const std::string& s) {
  if (s == "tanh" || s == "Tanh") return Activation::Tanh;
  return Activation::Relu;
}

Tensor2D activate(const Tensor2D& X, Activation a);
Tensor2D activate_grad(const Tensor2D& pre, Activation a);

} // namespace nwx
