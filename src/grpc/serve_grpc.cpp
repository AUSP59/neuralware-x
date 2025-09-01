// SPDX-License-Identifier: Apache-2.0
// NOTE: This target is optional and only built when NWX_ENABLE_GRPC=ON and gRPC+Protobuf are available.
#ifdef NWX_ENABLE_GRPC
#include <grpcpp/grpcpp.h>
#include "nwx/bundle.hpp"
#include "nwx/serialize.hpp"
#include "nwx/tensor.hpp"
#include "nwx/model.hpp"
#include "nwx.grpc.pb.h"
#include <memory>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class InferenceService final : public nwx::Inference::Service {
 public:
  explicit InferenceService(const std::string& bundle_path){
    if (!nwx::load_bundle(bundle_, bundle_path)) throw std::runtime_error("load_bundle failed");
    bundle_.model.temperature = 1.0;
  }
  Status Predict(ServerContext* context, const nwx::PredictRequest* req, nwx::PredictResponse* resp) override {
    // Unary predict (same as before)

    int rows = req->rows_size();
    if (rows==0) return Status::OK;
    int cols = req->rows(0).values_size();
    nwx::Tensor2D X(rows, cols);
    for (int i=0;i<rows;++i){ for (int j=0;j<cols;++j) X(i,j) = req->rows(i).values(j); }
    if (bundle_.has_scaler) X = bundle_.scaler.transform(X);
    auto P = nwx::softmax(bundle_.model.forward(X));
    for (int i=0;i<P.rows;++i){ auto* r = resp->add_probs(); for (int j=0;j<P.cols;++j) r->add_values(P(i,j)); }
    return Status::OK;
  }
 
  Status BatchPredict(ServerContext* context, const nwx::PredictRequest* req, nwx::PredictResponse* resp) override {
    return Predict(context, req, resp);
  }
  Status PredictStream(ServerContext* context, grpc::ServerReaderWriter<nwx::Row, nwx::Row>* stream) override {
    nwx::Row inrow;
    while (stream->Read(&inrow)) {
      int cols = inrow.values_size();
      if (cols<=0) { nwx::Row out; stream->Write(out); continue; }
      nwx::Tensor2D X(1, cols);
      for (int j=0;j<cols;++j) X(0,j) = inrow.values(j);
      if (bundle_.has_scaler) X = bundle_.scaler.transform(X);
      auto P = nwx::softmax(bundle_.model.forward(X));
      nwx::Row out; for (int j=0;j<P.cols;++j) out.add_values(P(0,j));
      stream->Write(out);
    }
    return Status::OK;
  }

private:
  nwx::ModelBundle bundle_;
};

int main(int argc, char** argv){
  if (argc<3){ std::cerr<<"Usage: nwx_grpc_serve <model.bundle> <port>\n"; return 2; }
  std::string bundle = argv[1]; std::string port = argv[2];
  std::string addr = "0.0.0.0:"+port;
  InferenceService svc(bundle);
  ServerBuilder builder; builder.AddListeningPort(addr, grpc::InsecureServerCredentials()); builder.RegisterService(&svc);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout<<"nwx_grpc_serve listening on "<<addr<<"\n";
  server->Wait();
  return 0;
}
#else
int main(){ return 0; }
#endif
