// SPDX-License-Identifier: Apache-2.0
#include "nwx/bundle.hpp"
#include "nwx/serialize.hpp"
#include "nwx/tensor.hpp"
#include "nwx/model.hpp"
#include <chrono>
#include <iostream>

using clock_t = std::chrono::steady_clock;
int main(int argc, char** argv){
  if (argc<3){ std::cerr<<"Usage: nwx_prof --model <bundle|bin> [--rows 2048] [--cols 16] [--runs 50]\n"; return 2; }
  std::string modelp; int rows=2048, cols=16, runs=50;
  for (int i=1;i<argc;++i){ std::string a=argv[i];
    if(a=="--model" && i+1<argc) modelp=argv[++i];
    else if(a=="--rows" && i+1<argc) rows=std::stoi(argv[++i]);
    else if(a=="--cols" && i+1<argc) cols=std::stoi(argv[++i]);
    else if(a=="--runs" && i+1<argc) runs=std::stoi(argv[++i]);
  }
  nwx::ModelBundle b; if(!nwx::load_bundle(b, modelp)){ if(!nwx::load_model(b.model, modelp)){ std::cerr<<"load failed\n"; return 3; } b.model.temperature=1.0; b.has_scaler=false; }
  nwx::Tensor2D X(rows, b.model.W1.cols); for (int i=0;i<rows;++i) for(int j=0;j<b.model.W1.cols;++j) X(i,j) = (i^j)%3 - 1.0;
  double best=1e9, sum=0.0;
  for (int r=0;r<runs; ++r){ auto t0=clock_t::now(); auto P = nwx::softmax(b.model.forward(X)); auto t1=clock_t::now(); double ms = std::chrono::duration<double, std::milli>(t1-t0).count(); best = std::min(best, ms); sum += ms; (void)P; }
  std::cout<<"runs="<<runs<<" avg_ms="<<(sum/runs)<<" best_ms="<<best<<"\n";
  return 0;
}
