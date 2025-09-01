// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>
namespace nwx {
struct CPUFeatures { bool avx2{false}; bool fma{false}; };
CPUFeatures detect_cpu();
} // namespace nwx
