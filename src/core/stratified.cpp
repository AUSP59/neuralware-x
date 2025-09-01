// SPDX-License-Identifier: Apache-2.0
#include "nwx/stratified.hpp"
#include <unordered_map>
#include <algorithm>

namespace nwx {
std::pair<std::vector<int>, std::vector<int>> stratified_split(const std::vector<int>& labels, double val_ratio, std::mt19937& rng) {
  std::unordered_map<int, std::vector<int>> byc;
  for (int i=0;i<(int)labels.size();++i) byc[labels[(size_t)i]].push_back(i);
  std::vector<int> tr, va;
  for (auto& kv : byc) {
    auto idx = kv.second;
    std::shuffle(idx.begin(), idx.end(), rng);
    int n = (int)idx.size();
    int n_val = std::clamp((int)(val_ratio*n), 0, n);
    va.insert(va.end(), idx.begin(), idx.begin()+n_val);
    tr.insert(tr.end(), idx.begin()+n_val, idx.end());
  }
  std::shuffle(tr.begin(), tr.end(), rng);
  std::shuffle(va.begin(), va.end(), rng);
  return {tr, va};
}
} // namespace nwx
