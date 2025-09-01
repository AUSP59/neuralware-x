// SPDX-License-Identifier: Apache-2.0
#include "nwx/tensor.hpp"
#include <algorithm>
#include "nwx/cpu_features.hpp"
#ifdef NWX_HAVE_OPENMP
#include <omp.h>
#endif
#ifdef NWX_HAVE_CUBLAS
extern "C" int nwx_cublas_dgemm(int m,int n,int k,const double* A,const double* B,double* C);
#endif

namespace nwx {

Tensor2D matmul(const Tensor2D& A, const Tensor2D& B) {
#ifdef NWX_ENABLE_AVX2
  auto feats = detect_cpu();
#endif
  /* AVX2 fast path */
  /* GPU path */

#ifdef NWX_ENABLE_AVX2
  if (feats.avx2 && A.cols == B.rows) {
    Tensor2D C(A.rows, B.cols);
    const int M=A.rows, N=B.cols, K=A.cols;
    const int BS = 64;
    for (int i0=0; i0<M; i0+=BS){
      int i1 = std::min(M, i0+BS);
      for (int j0=0; j0<N; j0+=BS){
        int j1 = std::min(N, j0+BS);
        for (int k0=0; k0<K; k0+=BS){
          int k1 = std::min(K, k0+BS);
          for (int i=i0; i<i1; ++i){
            for (int j=j0; j<j1; ++j){
              double acc = C(i,j);
              for (int k=k0; k<k1; ++k) acc += A(i,k)*B(k,j);
              C(i,j) = acc;
            }
          }
        }
      }
    }
    return C;
  }
#endif

#ifdef NWX_HAVE_CUBLAS
  if (A.cols == B.rows) {
    Tensor2D C(A.rows, B.cols);
    if (nwx_cublas_dgemm(A.rows, B.cols, A.cols, A.data.data(), B.data.data(), C.data.data()) == 0) return C;
  }
#endif
  assert(A.cols == B.rows);
  Tensor2D C(A.rows, B.cols);
  #ifdef NWX_USE_OPENMP
#pragma omp parallel for
#endif
  for (int i = 0; i < A.rows; ++i) {
    for (int k = 0; k < A.cols; ++k) {
      double a = A(i,k);
      for (int j = 0; j < B.cols; ++j) {
        C(i,j) += a * B(k,j);
      }
    }
  }
  return C;
}

Tensor2D add_row(const Tensor2D& A, const std::vector<double>& b) {
  assert(A.cols == static_cast<int>(b.size()));
  Tensor2D C = A;
  #ifdef NWX_USE_OPENMP
#pragma omp parallel for
#endif
  for (int i = 0; i < A.rows; ++i) {
    for (int j = 0; j < A.cols; ++j) {
      C(i,j) += b[static_cast<size_t>(j)];
    }
  }
  return C;
}

Tensor2D relu(const Tensor2D& X) {
  Tensor2D Y = X;
  for (int i = 0; i < X.rows; ++i) {
    for (int j = 0; j < X.cols; ++j) {
      if (Y(i,j) < 0.0) Y(i,j) = 0.0;
    }
  }
  return Y;
}

Tensor2D relu_grad(const Tensor2D& X) {
  Tensor2D G(X.rows, X.cols);
  for (int i = 0; i < X.rows; ++i) {
    for (int j = 0; j < X.cols; ++j) {
      G(i,j) = (X(i,j) > 0.0) ? 1.0 : 0.0;
    }
  }
  return G;
}

Tensor2D softmax(const Tensor2D& X) {
  Tensor2D Y(X.rows, X.cols);
  for (int i = 0; i < X.rows; ++i) {
    double maxv = -std::numeric_limits<double>::infinity();
    for (int j = 0; j < X.cols; ++j) maxv = std::max(maxv, X(i,j));
    double sum = 0.0;
    for (int j = 0; j < X.cols; ++j) { Y(i,j) = std::exp(X(i,j) - maxv); sum += Y(i,j); }
    for (int j = 0; j < X.cols; ++j) Y(i,j) /= sum;
  }
  return Y;
}

double cross_entropy(const Tensor2D& probs, const std::vector<int>& labels) {
  assert(probs.rows == static_cast<int>(labels.size()));
  double loss = 0.0;
  for (int i = 0; i < probs.rows; ++i) {
    int y = labels[static_cast<size_t>(i)];
    double p = std::max(probs(i,y), 1e-15);
    loss += -std::log(p);
  }
  return loss / static_cast<double>(probs.rows);
}

double accuracy(const Tensor2D& probs, const std::vector<int>& labels) {
  int correct = 0;
  for (int i = 0; i < probs.rows; ++i) {
    int best = 0; double bv = probs(i,0);
    for (int j = 1; j < probs.cols; ++j) if (probs(i,j) > bv) { bv = probs(i,j); best = j; }
    if (best == labels[static_cast<size_t>(i)]) ++correct;
  }
  return static_cast<double>(correct) / static_cast<double>(probs.rows);
}

void xavier_init(Tensor2D& W, std::mt19937& rng) {
  double scale = std::sqrt(6.0 / (W.rows + W.cols));
  std::uniform_real_distribution<double> dist(-scale, scale);
  for (int i = 0; i < W.rows; ++i)
    for (int j = 0; j < W.cols; ++j)
      W(i,j) = dist(rng);
}

} // namespace nwx
