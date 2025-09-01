// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "model.hpp"
#include <string>

namespace nwx {
bool save_model(const MLP& m, const std::string& path);
bool load_model(MLP& m, const std::string& path);
}
