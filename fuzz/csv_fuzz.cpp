// SPDX-License-Identifier: Apache-2.0
#include "nwx/csv.hpp"
#include <cstdint>
#include <cstddef>
#include <fstream>
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  // write to temp file
  std::string path = "fuzz_tmp.csv";
  std::ofstream out(path, std::ios::binary);
  out.write((const char*)Data, (std::streamsize)Size);
  out.close();
  try { auto rows = nwx::read_csv_numeric(path, ','); (void)rows; } catch (...) {}
  std::remove(path.c_str());
  return 0;
}
