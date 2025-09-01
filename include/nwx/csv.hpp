// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <string>
#include <vector>

namespace nwx {
std::vector<std::vector<double>> read_csv_numeric(const std::string& path, char sep=',');
}
