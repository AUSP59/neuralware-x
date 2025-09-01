// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "model.hpp"
#include "scaler.hpp"

namespace nwx {
struct ModelBundle {
  MLP model;
  bool has_scaler{false};
  StandardScaler scaler;
};
bool save_bundle(const ModelBundle& b, const std::string& path);
bool load_bundle(ModelBundle& b, const std::string& path);
}
