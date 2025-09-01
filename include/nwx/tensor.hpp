// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <vector>
#include <random>
#include <string>
#include <cassert>
#include <cmath>
#include <limits>

namespace nwx {

struct Tensor2D {
  int rows{0}, cols{0};
  std::vector<double> data; // row-major
  Tensor2D() = default;
  Tensor2D(int r, int c) : rows(r), cols(c), data(static_cast<size_t>(r*c), 0.0) {}
  double& operator()(int r, int c) { return data[static_cast<size_t>(r*cols + c)]; }
  double operator()(int r, int c) const { return data[static_cast<size_t>(r*cols + c)]; }
};

Tensor2D matmul(const Tensor2D& A, const Tensor2D& B);
Tensor2D add_row(const Tensor2D& A, const std::vector<double>& b); // add bias row-wise
Tensor2D relu(const Tensor2D& X);
Tensor2D relu_grad(const Tensor2D& X);
Tensor2D softmax(const Tensor2D& X);
double cross_entropy(const Tensor2D& probs, const std::vector<int>& labels);
double accuracy(const Tensor2D& probs, const std::vector<int>& labels);
void xavier_init(Tensor2D& W, std::mt19937& rng);

} // namespace nwx
