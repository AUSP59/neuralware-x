// SPDX-License-Identifier: Apache-2.0
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "nwx/model.hpp"
#include "nwx/tensor.hpp"

namespace py = pybind11;
using namespace nwx;

PYBIND11_MODULE(nwxpy, m) {
  py::class_<MLP>(m, "MLP")
    .def(py::init<>())
    .def("init", [](MLP& self, int in, int hidden, int out, int seed){
      std::mt19937 rng(seed); self.init(in, hidden, out, rng);
    })
    .def("forward", [](MLP& self, const std::vector<std::vector<double>>& xs){
      int rows = (int)xs.size(); int cols = rows? (int)xs[0].size():0;
      Tensor2D X(rows, cols); for (int i=0;i<rows;++i) for (int j=0;j<cols;++j) X(i,j)=xs[i][j];
      auto P = softmax(self.forward(X));
      std::vector<std::vector<double>> out(rows, std::vector<double>(P.cols));
      for (int i=0;i<rows;++i) for (int j=0;j<P.cols;++j) out[i][j]=P(i,j);
      return out;
    });
}
