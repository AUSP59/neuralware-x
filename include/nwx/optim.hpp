// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tensor.hpp"
#include <vector>

namespace nwx {

struct AdamState {
  Tensor2D mW1, vW1, mW2, vW2;
  std::vector<double> mb1, vb1, mb2, vb2;
  int t{0};
};

void sgd_update(Tensor2D& W1, std::vector<double>& b1,
                Tensor2D& W2, std::vector<double>& b2,
                const Tensor2D& gW1, const std::vector<double>& gb1,
                const Tensor2D& gW2, const std::vector<double>& gb2,
                double lr, double wd);

void adam_update(AdamState& st,
                 Tensor2D& W1, std::vector<double>& b1,
                 Tensor2D& W2, std::vector<double>& b2,
                 const Tensor2D& gW1, const std::vector<double>& gb1,
                 const Tensor2D& gW2, const std::vector<double>& gb2,
                 double lr, double wd, double beta1=0.9, double beta2=0.999, double eps=1e-8);

} // namespace nwx
