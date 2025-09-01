// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tensor.hpp"
#include <vector>
#include <tuple>

namespace nwx {
std::tuple<double,double> f1_macro_micro(const Tensor2D& probs, const std::vector<int>& labels, int n_classes);
double pr_auc_binary(const Tensor2D& probs, const std::vector<int>& labels);
}
