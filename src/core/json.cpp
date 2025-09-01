// SPDX-License-Identifier: Apache-2.0
#include "nwx/json.hpp"
#include <stdexcept>

namespace nwx {

static void skip_ws(const std::string& s, size_t& i) {
  while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
}

bool JsonFlat::parse(const std::string& text) {
  kv_.clear();
  size_t i = 0;
  skip_ws(text, i);
  if (i >= text.size() || text[i] != '{') return false;
  ++i;
  while (i < text.size()) {
    skip_ws(text, i);
    if (i < text.size() && text[i] == '}') { ++i; break; }
    if (text[i] != '"') return false;
    ++i;
    size_t start = i;
    while (i < text.size() && text[i] != '"') ++i;
    if (i >= text.size()) return false;
    std::string key = text.substr(start, i-start);
    ++i;
    skip_ws(text, i);
    if (i >= text.size() || text[i] != ':') return false;
    ++i;
    skip_ws(text, i);
    if (i >= text.size()) return false;
    if (text[i] == '"') {
      ++i; size_t s2 = i;
      while (i < text.size() && text[i] != '"') ++i;
      if (i >= text.size()) return false;
      std::string val = text.substr(s2, i-s2);
      ++i;
      kv_[key] = val;
    } else if (std::isdigit(static_cast<unsigned char>(text[i])) || text[i]=='-' || text[i]=='+') {
      size_t s2 = i;
      while (i < text.size() && (std::isdigit(static_cast<unsigned char>(text[i])) || text[i]=='.' || text[i]=='e' || text[i]=='E' || text[i]=='-' || text[i]=='+')) ++i;
      double num = std::stod(text.substr(s2, i-s2));
      kv_[key] = num;
    } else if (!text.compare(i, 4, "true")) { i += 4; kv_[key] = true; }
    else if (!text.compare(i, 5, "false")) { i += 5; kv_[key] = false; }
    else {
      return false;
    }
    skip_ws(text, i);
    if (i < text.size() && text[i] == ',') { ++i; continue; }
    if (i < text.size() && text[i] == '}') { ++i; break; }
  }
  return true;
}

std::optional<std::string> JsonFlat::get_string(const std::string& k) const {
  auto it = kv_.find(k);
  if (it == kv_.end()) return std::nullopt;
  if (auto p = std::get_if<std::string>(&it->second)) return *p;
  return std::nullopt;
}

std::optional<double> JsonFlat::get_number(const std::string& k) const {
  auto it = kv_.find(k);
  if (it == kv_.end()) return std::nullopt;
  if (auto p = std::get_if<double>(&it->second)) return *p;
  return std::nullopt;
}

std::optional<bool> JsonFlat::get_bool(const std::string& k) const {
  auto it = kv_.find(k);
  if (it == kv_.end()) return std::nullopt;
  if (auto p = std::get_if<bool>(&it->second)) return *p;
  return std::nullopt;
}

} // namespace nwx
