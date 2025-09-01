// SPDX-License-Identifier: Apache-2.0
#include <benchmark/benchmark.h>
#include <vector>
static void BM_VectorPushBack(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<int> v;
    for (int i = 0; i < 1000; ++i) v.push_back(i);
    benchmark::DoNotOptimize(v.data());
  }
}
BENCHMARK(BM_VectorPushBack);
BENCHMARK_MAIN();
