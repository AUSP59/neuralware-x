// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "tensor.hpp"
#include <vector>
#include <utility>

namespace nwx {
std::vector<std::vector<int>> confusion_matrix(const Tensor2D& probs, const std::vector<int>& labels, int n_classes);
std::pair<double,double> precision_recall(const Tensor2D& probs, const std::vector<int>& labels, int positive_class);
double f1_score(const Tensor2D& probs, const std::vector<int>& labels, int positive_class);
}
