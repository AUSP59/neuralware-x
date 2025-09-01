// SPDX-License-Identifier: Apache-2.0
#define NWX_CAPI_EXPORT
#include "nwx/capi.h"
#include "nwx/bundle.hpp"
#include "nwx/serialize.hpp"
#include "nwx/tensor.hpp"
#include "nwx/model.hpp"
#include <memory>
using namespace nwx;
struct Handle { ModelBundle b; };
extern "C" {
nwx_handle nwx_load_bundle(const char* path){
  if (!path) return nullptr;
  std::unique_ptr<Handle> h(new Handle());
  if (!load_bundle(h->b, std::string(path))) return nullptr;
  h->b.model.temperature = 1.0;
  return (nwx_handle)h.release();
}
int nwx_output_dim(nwx_handle hh){ if(!hh) return -1; auto* h=(Handle*)hh; return h->b.model.W2.cols; }
int nwx_predict(nwx_handle hh, const double* data, int rows, int cols, double* out_probs){
  if (!hh || !data || rows<=0 || cols<=0 || !out_probs) return -1;
  auto* h=(Handle*)hh; Tensor2D X(rows, cols);
  for (int i=0;i<rows;++i) for (int j=0;j<cols;++j) X(i,j)=data[i*cols+j];
  if (h->b.has_scaler) X=h->b.scaler.transform(X);
  auto P = softmax(h->b.model.forward(X));
  for (int i=0;i<P.rows;++i) for (int j=0;j<P.cols;++j) out_probs[i*P.cols+j]=P(i,j);
  return 0;
}
void nwx_free(nwx_handle hh){ if(!hh) return; delete (Handle*)hh; }
}
