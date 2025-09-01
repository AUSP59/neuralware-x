// SPDX-License-Identifier: Apache-2.0
#include "nwx/tensor.hpp"
#include "nwx/metrics_ex.hpp"
#include "nwx/roc_auc.hpp"
#include "nwx/calibration.hpp"
#include <cassert>
int main(){
  nwx::Tensor2D P(4,2);
  // simple separable probs
  P(0,0)=0.9; P(0,1)=0.1;
  P(1,0)=0.8; P(1,1)=0.2;
  P(2,0)=0.2; P(2,1)=0.8;
  P(3,0)=0.1; P(3,1)=0.9;
  std::vector<int> y={0,0,1,1};
  auto f1 = nwx::f1_macro_micro(P, y, 2);
  assert(std::get<0>(f1) > 0.9 && std::get<1>(f1) > 0.9);
  double auc = nwx::roc_auc_binary(P, y);
  assert(auc > 0.95);
  double ece = nwx::expected_calibration_error(P, y, 5);
  assert(ece >= 0.0);
  return 0;
}
