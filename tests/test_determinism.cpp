// SPDX-License-Identifier: Apache-2.0
#include "nwx/trainer.hpp"
#include "nwx/tensor.hpp"
#include <cassert>
int main(){
  // Tiny deterministic dataset
  nwx::Tensor2D X(4,2);
  X(0,0)=0;X(0,1)=0; X(1,0)=0;X(1,1)=1; X(2,0)=1;X(2,1)=0; X(3,0)=1;X(3,1)=1;
  std::vector<int> y={0,1,1,0}; int C=2;
  nwx::Dataset ds{X,y,C};
  nwx::TrainConfig cfg; cfg.epochs=100; cfg.optimizer="adam"; cfg.seed=123; cfg.val_split=0.25; cfg.batch=2; cfg.patience=10;
  std::mt19937 r1(cfg.seed), r2(cfg.seed);
  nwx::MLP m1; m1.init(2,8,2,r1);
  nwx::MLP m2; m2.init(2,8,2,r2);
  nwx::train_xent(m1, ds, cfg);
  nwx::train_xent(m2, ds, cfg);
  // Models should be identical with same seed and config
  assert(m1.W1.data.size()==m2.W1.data.size());
  for (size_t i=0;i<m1.W1.data.size();++i) assert(std::abs(m1.W1.data[i]-m2.W1.data[i])<1e-12);
  return 0;
}
