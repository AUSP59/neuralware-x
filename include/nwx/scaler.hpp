// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tensor.hpp"
#include <vector>

namespace nwx {

struct StandardScaler {
  std::vector<double> mean, stdv;
  void fit(const Tensor2D& X);
  Tensor2D transform(const Tensor2D& X) const;
};

} // namespace nwx
