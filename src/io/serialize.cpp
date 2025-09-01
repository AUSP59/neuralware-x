// SPDX-License-Identifier: Apache-2.0
#include "nwx/serialize.hpp"
#include <fstream>

namespace nwx {

bool save_model(const MLP& m, const std::string& path) {
  std::ofstream out(path, std::ios::binary);
  if (!out) return false;
  out.write((char*)&m.in, sizeof(m.in));
  out.write((char*)&m.hidden, sizeof(m.hidden));
  out.write((char*)&m.out, sizeof(m.out));
  auto write_vec = [&](const std::vector<double>& v) {
    size_t n = v.size();
    out.write((char*)&n, sizeof(n));
    out.write((char*)v.data(), static_cast<std::streamsize>(n*sizeof(double)));
  };
  auto write_mat = [&](const Tensor2D& t) {
    out.write((char*)&t.rows, sizeof(t.rows));
    out.write((char*)&t.cols, sizeof(t.cols));
    size_t n = t.data.size();
    out.write((char*)t.data.data(), static_cast<std::streamsize>(n*sizeof(double)));
  };
  write_mat(m.W1); write_vec(m.b1);
  write_mat(m.W2); write_vec(m.b2);
  return true;
}

bool load_model(MLP& m, const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return false;
  in.read((char*)&m.in, sizeof(m.in));
  in.read((char*)&m.hidden, sizeof(m.hidden));
  in.read((char*)&m.out, sizeof(m.out));
  auto read_vec = [&](std::vector<double>& v) {
    size_t n = 0; in.read((char*)&n, sizeof(n));
    v.assign(n, 0.0);
    in.read((char*)v.data(), static_cast<std::streamsize>(n*sizeof(double)));
  };
  auto read_mat = [&](Tensor2D& t) {
    in.read((char*)&t.rows, sizeof(t.rows));
    in.read((char*)&t.cols, sizeof(t.cols));
    t.data.assign(static_cast<size_t>(t.rows*t.cols), 0.0);
    in.read((char*)t.data.data(), static_cast<std::streamsize>(t.data.size()*sizeof(double)));
  };
  read_mat(m.W1); read_vec(m.b1);
  read_mat(m.W2); read_vec(m.b2);
  return true;
}

} // namespace nwx
