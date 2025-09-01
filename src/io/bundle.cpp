// SPDX-License-Identifier: Apache-2.0
#include "nwx/bundle.hpp"
#include "nwx/serialize.hpp"
#include <fstream>
#include <cstring>

namespace nwx {

// New format: magic "NWX2", then activation, dims, weights, biases, scaler flag+stats
static const char MAGIC[4] = {'N','W','X','3'};

bool save_bundle(const ModelBundle& b, const std::string& path) {
  std::ofstream out(path, std::ios::binary);
  if (!out) return false;
  out.write(MAGIC, 4);
  int act = static_cast<int>(b.model.act);
  out.write((char*)&act, sizeof(act));
  // reuse existing save_model for tensors, but we also need dims and weights; call into save_model, then append scaler
  // To keep compatibility, write the same layout used by save_model after the header.
  out.write((char*)&b.model.in, sizeof(b.model.in));
  out.write((char*)&b.model.hidden, sizeof(b.model.hidden));
  out.write((char*)&b.model.out, sizeof(b.model.out));
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
  write_mat(b.model.W1); write_vec(b.model.b1);
  write_mat(b.model.W2); write_vec(b.model.b2);
  // temperature
  double temp = b.model.temperature;
  out.write((char*)&temp, sizeof(temp));
  // scaler
  int has = b.has_scaler ? 1 : 0;
  out.write((char*)&has, sizeof(has));
  if (b.has_scaler) {
    size_t n = b.scaler.mean.size();
    out.write((char*)&n, sizeof(n));
    out.write((char*)b.scaler.mean.data(), (std::streamsize)(n*sizeof(double)));
    out.write((char*)b.scaler.stdv.data(), (std::streamsize)(n*sizeof(double)));
  }
  return true;
}

bool load_bundle(ModelBundle& b, const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return false;
  char hdr[4] = {0,0,0,0};
  in.read(hdr, 4);
  if (in.gcount() == 4 && std::memcmp(hdr, MAGIC, 4) == 0) {
    int act=0; in.read((char*)&act, sizeof(act));
    b.model.act = static_cast<Activation>(act);
    in.read((char*)&b.model.in, sizeof(b.model.in));
    in.read((char*)&b.model.hidden, sizeof(b.model.hidden));
    in.read((char*)&b.model.out, sizeof(b.model.out));
    auto read_vec = [&](std::vector<double>& v) {
      size_t n = 0; in.read((char*)&n, sizeof(n));
      v.assign(n, 0.0);
      in.read((char*)v.data(), static_cast<std::streamsize>(n*sizeof(double)));
    };
    auto read_mat = [&](Tensor2D& t) {
      in.read((char*)&t.rows, sizeof(t.rows));
      in.read((char*)&t.cols, sizeof(t.cols));
      t.data.assign((size_t)(t.rows*t.cols), 0.0);
      in.read((char*)t.data.data(), static_cast<std::streamsize>(t.data.size()*sizeof(double)));
    };
    read_mat(b.model.W1); read_vec(b.model.b1);
    read_mat(b.model.W2); read_vec(b.model.b2);
    // temperature
    double temp = 1.0; in.read((char*)&temp, sizeof(temp)); b.model.temperature = temp;
    int has=0; in.read((char*)&has, sizeof(has));
    b.has_scaler = (has==1);
    if (b.has_scaler) {
      size_t n=0; in.read((char*)&n, sizeof(n));
      b.scaler.mean.assign(n, 0.0); b.scaler.stdv.assign(n, 1.0);
      in.read((char*)b.scaler.mean.data(), (std::streamsize)(n*sizeof(double)));
      in.read((char*)b.scaler.stdv.data(), (std::streamsize)(n*sizeof(double)));
    }
    return true;
  } else {
    // fallback to old format (no header)
    in.clear(); in.seekg(0);
    if (!load_model(b.model, path)) return false;
    b.has_scaler = false;
    return true;
  }
}

} // namespace nwx
