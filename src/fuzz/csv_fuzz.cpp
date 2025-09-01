// SPDX-License-Identifier: Apache-2.0
#include "nwx/csv.hpp"
#include <cstdint>
#include <cstddef>
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  std::string s((const char*)Data, (const char*)Data + Size);
  try { auto rows = nwx::read_csv_numeric_string(s, ','); (void)rows.size(); } catch(...) {}
  return 0;
}
