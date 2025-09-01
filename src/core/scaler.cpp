// SPDX-License-Identifier: Apache-2.0
#include "nwx/scaler.hpp"
#include <cmath>

namespace nwx {

void StandardScaler::fit(const Tensor2D& X) {
  mean.assign(X.cols, 0.0);
  stdv.assign(X.cols, 0.0);
  for (int j = 0; j < X.cols; ++j) {
    double s=0.0; for (int i = 0; i < X.rows; ++i) s += X(i,j);
    mean[j] = s / X.rows;
    double v=0.0; for (int i = 0; i < X.rows; ++i) { double d = X(i,j)-mean[j]; v += d*d; }
    stdv[j] = std::sqrt(v / X.rows);
    if (stdv[j] == 0.0) stdv[j] = 1.0;
  }
}
Tensor2D StandardScaler::transform(const Tensor2D& X) const {
  Tensor2D Z(X.rows, X.cols);
  for (int i = 0; i < X.rows; ++i)
    for (int j = 0; j < X.cols; ++j)
      Z(i,j) = (X(i,j) - mean[j]) / stdv[j];
  return Z;
}

} // namespace nwx
