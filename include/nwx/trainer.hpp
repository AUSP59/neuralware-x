// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "model.hpp"
#include "tensor.hpp"
#include <string>
#include <random>

namespace nwx {

struct TrainConfig {
  int epochs{1000};
  double lr{0.1};
  int hidden{8};
  int seed{42};
  double weight_decay{0.0};
  int batch{0}; // 0 = full batch
  double val_split{0.0};
  int patience{0}; // early stopping patience (epochs), 0=disabled
  bool standardize{false};
  std::string optimizer{"sgd"};
  std::string lr_schedule{"none"};
  int warmup_steps{0};
  double clip_norm{0.0};
  std::string history_path;
  std::string activation{"relu"};
  int epochs{1000};
  double lr{0.1};
  int hidden{8};
  int seed{42};
};

struct Dataset {
  Tensor2D X;
  std::vector<int> y;
  int n_classes{0};
};

Dataset load_csv_classification(const std::string& path); // last col is int label
void train_xent(MLP& model, const Dataset& data, const TrainConfig& cfg);

} // namespace nwx
