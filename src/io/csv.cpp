// SPDX-License-Identifier: Apache-2.0
#include "nwx/csv.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace nwx {

std::vector<std::vector<double>> read_csv_numeric(const std::string& path, char sep) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("Failed to open CSV: " + path);
  std::vector<std::vector<double>> rows;
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty()) continue;
    std::stringstream ss(line);
    std::string cell;
    std::vector<double> vals;
    while (std::getline(ss, cell, sep)) {
      vals.push_back(std::stod(cell));
    }
    rows.push_back(std::move(vals));
  }
  return rows;
}

} // namespace nwx
