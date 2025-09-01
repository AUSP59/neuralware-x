// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <string>
#include <unordered_map>
#include <variant>
#include <optional>
#include <cctype>

namespace nwx {

// Minimal JSON (flat object) parser for strings, numbers, booleans.
class JsonFlat {
 public:
  bool parse(const std::string& text);
  std::optional<std::string> get_string(const std::string& k) const;
  std::optional<double> get_number(const std::string& k) const;
  std::optional<bool> get_bool(const std::string& k) const;
 private:
  std::unordered_map<std::string, std::variant<std::string,double,bool>> kv_;
};

} // namespace nwx
