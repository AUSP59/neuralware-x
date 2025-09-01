// SPDX-License-Identifier: Apache-2.0
#include "nwx/json.hpp"
#include <cstdint>
#include <cstddef>
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  std::string s((const char*)Data, (const char*)Data + Size);
  nwx::JsonFlat jf; jf.parse(s);
  return 0;
}
