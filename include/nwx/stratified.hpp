// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <vector>
#include <random>

namespace nwx {
std::pair<std::vector<int>, std::vector<int>> stratified_split(const std::vector<int>& labels, double val_ratio, std::mt19937& rng);
}
