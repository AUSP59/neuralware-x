// SPDX-License-Identifier: Apache-2.0
#include "nwx/quant.hpp"
#include <cmath>
#ifdef NWX_HAVE_OPENMP
#include <omp.h>
#endif

namespace nwx {

Tensor2D gemm_i8_dequant(const Tensor2D& X, const QuantLayer& ql, double sx){
  int rows = X.rows, k = X.cols, out = ql.out;
  Tensor2D Y(rows, out);
  // Precompute inv scales
  const double sxf = sx;
#ifdef NWX_HAVE_OPENMP
#pragma omp parallel for
#endif
  for (int r=0; r<rows; ++r){
    for (int o=0; o<out; ++o){
      const int8_t* wrow = &ql.Wq[(size_t)o * ql.in];
      long long acc = 0;
      for (int i=0;i<k;++i){
        acc += (long long)wrow[i] * (long long)std::lround(X(r,i)/sxf);
      }
      double y = (double)acc * (sxf * ql.sw[o]);
      Y(r,o) = y;
    }
  }
  return Y;
}

} // namespace nwx
