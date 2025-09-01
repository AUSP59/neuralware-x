// SPDX-License-Identifier: Apache-2.0
#include "nwx/trainer.hpp"
#include "nwx/csv.hpp"
#include <stdexcept>

namespace nwx {

Dataset load_csv_classification(const std::string& path) {
  auto rows = read_csv_numeric(path, ',');
  if (rows.empty()) throw std::runtime_error("Empty dataset: " + path);
  int in_dim = (int)rows[0].size() - 1;
  int out_classes = 0;
  std::vector<int> y;
  Tensor2D X((int)rows.size(), in_dim);
  for (int i = 0; i < (int)rows.size(); ++i) {
    for (int j = 0; j < in_dim; ++j) X(i,j) = rows[i][j];
    int label = (int)rows[i][in_dim];
    if (label+1 > out_classes) out_classes = label+1;
    y.push_back(label);
  }
  return Dataset{X, y, out_classes};
}

} // namespace nwx
