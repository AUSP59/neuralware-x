// SPDX-License-Identifier: Apache-2.0
#include "nwx/optim.hpp"
#include <cmath>

namespace nwx {

void sgd_update(Tensor2D& W1, std::vector<double>& b1,
                Tensor2D& W2, std::vector<double>& b2,
                const Tensor2D& gW1, const std::vector<double>& gb1,
                const Tensor2D& gW2, const std::vector<double>& gb2,
                double lr, double wd) {
  for (int i = 0; i < W1.rows; ++i) for (int j = 0; j < W1.cols; ++j)
    W1(i,j) -= lr * (gW1(i,j) + wd * W1(i,j));
  for (size_t j = 0; j < b1.size(); ++j) b1[j] -= lr * gb1[j];

  for (int i = 0; i < W2.rows; ++i) for (int j = 0; j < W2.cols; ++j)
    W2(i,j) -= lr * (gW2(i,j) + wd * W2(i,j));
  for (size_t j = 0; j < b2.size(); ++j) b2[j] -= lr * gb2[j];
}

void adam_update(AdamState& st,
                 Tensor2D& W1, std::vector<double>& b1,
                 Tensor2D& W2, std::vector<double>& b2,
                 const Tensor2D& gW1, const std::vector<double>& gb1,
                 const Tensor2D& gW2, const std::vector<double>& gb2,
                 double lr, double wd, double beta1, double beta2, double eps) {
  if (st.t == 0) {
    st.mW1 = Tensor2D(W1.rows, W1.cols); st.vW1 = Tensor2D(W1.rows, W1.cols);
    st.mW2 = Tensor2D(W2.rows, W2.cols); st.vW2 = Tensor2D(W2.rows, W2.cols);
    st.mb1.assign(b1.size(), 0.0); st.vb1.assign(b1.size(), 0.0);
    st.mb2.assign(b2.size(), 0.0); st.vb2.assign(b2.size(), 0.0);
  }
  st.t += 1;
  auto upd_mat = [&](Tensor2D& W, const Tensor2D& g, Tensor2D& m, Tensor2D& v) {
    for (int i = 0; i < W.rows; ++i) {
      for (int j = 0; j < W.cols; ++j) {
        double gi = g(i,j) + wd * W(i,j);
        m(i,j) = beta1*m(i,j) + (1-beta1)*gi;
        v(i,j) = beta2*v(i,j) + (1-beta2)*gi*gi;
        double mhat = m(i,j) / (1-std::pow(beta1, st.t));
        double vhat = v(i,j) / (1-std::pow(beta2, st.t));
        W(i,j) -= lr * (mhat / (std::sqrt(vhat) + eps));
      }
    }
  };
  auto upd_vec = [&](std::vector<double>& W, const std::vector<double>& g, std::vector<double>& m, std::vector<double>& v) {
    for (size_t j = 0; j < W.size(); ++j) {
      double gi = g[j];
      m[j] = beta1*m[j] + (1-beta1)*gi;
      v[j] = beta2*v[j] + (1-beta2)*gi*gi;
      double mhat = m[j] / (1-std::pow(beta1, st.t));
      double vhat = v[j] / (1-std::pow(beta2, st.t));
      W[j] -= lr * (mhat / (std::sqrt(vhat) + eps));
    }
  };
  upd_mat(W1, gW1, st.mW1, st.vW1);
  upd_vec(b1, gb1, st.mb1, st.vb1);
  upd_mat(W2, gW2, st.mW2, st.vW2);
  upd_vec(b2, gb2, st.mb2, st.vb2);
}

} // namespace nwx
