// SPDX-License-Identifier: Apache-2.0
#include "nwx/cpu_features.hpp"
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif

namespace nwx {
CPUFeatures detect_cpu(){
  CPUFeatures f{};
#ifdef __x86_64__
  unsigned int eax, ebx, ecx, edx;
  unsigned int leaf7_ebx=0, leaf1_ecx=0;
#ifdef _MSC_VER
  int info[4];
  __cpuid(info, 1); leaf1_ecx = (unsigned)info[2];
  __cpuid(info, 7); leaf7_ebx = (unsigned)info[1];
#else
  __get_cpuid(1, &eax, &ebx, &ecx, &edx); leaf1_ecx = ecx;
  unsigned int _ecx, _edx;
  __get_cpuid_count(7, 0, &eax, &leaf7_ebx, &_ecx, &_edx);
#endif
  f.avx2 = (leaf7_ebx & (1u<<5)) != 0; // AVX2
  f.fma = (leaf1_ecx & (1u<<12)) != 0; // FMA
#endif
  return f;
}
} // namespace nwx
