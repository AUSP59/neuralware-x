// SPDX-License-Identifier: Apache-2.0
#include "nwx/split.hpp"
#include <algorithm>

namespace nwx {

std::pair<std::vector<int>, std::vector<int>> train_val_split(int n, double val_ratio, std::mt19937& rng) {
  std::vector<int> idx(n);
  for (int i = 0; i < n; ++i) idx[i] = i;
  std::shuffle(idx.begin(), idx.end(), rng);
  int n_val = static_cast<int>(val_ratio * n);
  if (n_val < 0) n_val = 0;
  if (n_val > n) n_val = n;
  std::vector<int> val(idx.begin(), idx.begin() + n_val);
  std::vector<int> trn(idx.begin() + n_val, idx.end());
  return {trn, val};
}

} // namespace nwx
