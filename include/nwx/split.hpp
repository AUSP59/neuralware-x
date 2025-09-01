// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <vector>
#include <random>

namespace nwx {
std::pair<std::vector<int>, std::vector<int>> train_val_split(int n, double val_ratio, std::mt19937& rng);
}
