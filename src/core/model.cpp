// SPDX-License-Identifier: Apache-2.0
#include "nwx/model.hpp"
#include "nwx/activation.hpp"
#include <algorithm>

namespace nwx {

void MLP::init(int in_dim, int hidden_dim, int out_dim, std::mt19937& rng, Activation a) {
  in = in_dim; hidden = hidden_dim; out = out_dim;
  W1 = Tensor2D(in, hidden);
  W2 = Tensor2D(hidden, out);
  b1.assign(static_cast<size_t>(hidden), 0.0);
  b2.assign(static_cast<size_t>(out), 0.0);
  act = a;
  xavier_init(W1, rng);
  xavier_init(W2, rng);
}

Tensor2D MLP::forward(const Tensor2D& X) {
  x1 = add_row(matmul(X, W1), b1);   // logits of layer 1 pre-activation
  a1 = activate(x1, act);
  a2 = add_row(matmul(a1, W2), b2);  // logits
  return a2;
}

void MLP::backward(const Tensor2D& X, const std::vector<int>& y, double lr) {
  // Forward (uses cached a2 from last forward())
  Tensor2D probs = softmax(a2);
  // Gradient on logits (softmax+CE)
  Tensor2D dlogits = probs;
  for (int i = 0; i < dlogits.rows; ++i) dlogits(i, y[static_cast<size_t>(i)]) -= 1.0;
  for (double& v : dlogits.data) v /= static_cast<double>(dlogits.rows);

  // Grad W2, b2
  Tensor2D a1T(a1.cols, a1.rows);
  for (int i = 0; i < a1.rows; ++i) for (int j = 0; j < a1.cols; ++j) a1T(j,i) = a1(i,j);
  Tensor2D gW2 = matmul(a1T, dlogits);
  std::vector<double> gb2(static_cast<size_t>(b2.size()), 0.0);
  for (int i = 0; i < dlogits.rows; ++i) for (int j = 0; j < dlogits.cols; ++j) gb2[static_cast<size_t>(j)] += dlogits(i,j);

  // Grad through ReLU
  Tensor2D d_a1(a1.rows, a1.cols);
  for (int i = 0; i < a1.rows; ++i) {
    for (int j = 0; j < a1.cols; ++j) {
      double sum = 0.0;
      for (int k = 0; k < dlogits.cols; ++k) sum += dlogits(i,k) * W2(j,k);
      double g = activate_grad(x1, act)(i,j);
      d_a1(i,j) = g * sum;
    }
  }

  // Grad W1, b1
  Tensor2D XT(X.cols, X.rows);
  for (int i = 0; i < X.rows; ++i) for (int j = 0; j < X.cols; ++j) XT(j,i) = X(i,j);
  Tensor2D gW1 = matmul(XT, d_a1);
  std::vector<double> gb1(static_cast<size_t>(b1.size()), 0.0);
  for (int i = 0; i < d_a1.rows; ++i) for (int j = 0; j < d_a1.cols; ++j) gb1[static_cast<size_t>(j)] += d_a1(i,j);

  // SGD step
  for (int i = 0; i < W2.rows; ++i) for (int j = 0; j < W2.cols; ++j) W2(i,j) -= lr * gW2(i,j);
  for (size_t j = 0; j < b2.size(); ++j) b2[j] -= lr * gb2[j];
  for (int i = 0; i < W1.rows; ++i) for (int j = 0; j < W1.cols; ++j) W1(i,j) -= lr * gW1(i,j);
  for (size_t j = 0; j < b1.size(); ++j) b1[j] -= lr * gb1[j];
}

} // namespace nwx
