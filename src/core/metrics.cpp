// SPDX-License-Identifier: Apache-2.0
#include "nwx/metrics.hpp"
#include <algorithm>

namespace nwx {

std::vector<std::vector<int>> confusion_matrix(const Tensor2D& probs, const std::vector<int>& labels, int n_classes) {
  std::vector<std::vector<int>> cm(n_classes, std::vector<int>(n_classes, 0));
  for (int i = 0; i < probs.rows; ++i) {
    int pred = 0; double bv = probs(i,0);
    for (int j = 1; j < probs.cols; ++j) if (probs(i,j) > bv) { bv = probs(i,j); pred = j; }
    int y = labels[static_cast<size_t>(i)];
    cm[y][pred] += 1;
  }
  return cm;
}

std::pair<double,double> precision_recall(const Tensor2D& probs, const std::vector<int>& labels, int positive_class) {
  int tp=0, fp=0, fn=0;
  for (int i = 0; i < probs.rows; ++i) {
    int pred = 0; double bv = probs(i,0);
    for (int j = 1; j < probs.cols; ++j) if (probs(i,j) > bv) { bv = probs(i,j); pred = j; }
    int y = labels[static_cast<size_t>(i)];
    if (pred == positive_class && y == positive_class) ++tp;
    else if (pred == positive_class && y != positive_class) ++fp;
    else if (pred != positive_class && y == positive_class) ++fn;
  }
  double prec = tp + fp == 0 ? 0.0 : (double)tp/(tp+fp);
  double rec  = tp + fn == 0 ? 0.0 : (double)tp/(tp+fn);
  return {prec, rec};
}

double f1_score(const Tensor2D& probs, const std::vector<int>& labels, int positive_class) {
  auto pr = precision_recall(probs, labels, positive_class);
  double p = pr.first, r = pr.second;
  return (p+r)==0.0?0.0:2.0*p*r/(p+r);
}

} // namespace nwx
