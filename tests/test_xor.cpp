// SPDX-License-Identifier: Apache-2.0
#include "nwx/trainer.hpp"
#include "nwx/model.hpp"
#include "nwx/tensor.hpp"
#include <random>
#include "minitest.hpp"

using namespace nwx;

TEST(MLP_learns_XOR) {
  // XOR dataset: 4 samples, inputs in {0,1}, labels {0,1}
  Tensor2D X(4,2);
  X(0,0)=0; X(0,1)=0;
  X(1,0)=0; X(1,1)=1;
  X(2,0)=1; X(2,1)=0;
  X(3,0)=1; X(3,1)=1;
  std::vector<int> y = {0,1,1,0};
  Dataset ds{X,y,2};
  TrainConfig cfg; cfg.epochs=2000; cfg.lr=0.1; cfg.hidden=4; cfg.seed=7;
  std::mt19937 rng(cfg.seed);
  MLP m; m.init(2, cfg.hidden, 2, rng);
  train_xent(m, ds, cfg);
  auto probs = softmax(m.forward(ds.X));
  double acc = accuracy(probs, ds.y);
  EXPECT_GE(acc, 0.99);
}
