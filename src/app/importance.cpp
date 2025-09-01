// SPDX-License-Identifier: Apache-2.0
#include "nwx/bundle.hpp"
#include "nwx/serialize.hpp"
#include "nwx/csv.hpp"
#include "nwx/tensor.hpp"
#include "nwx/metrics.hpp"
#include <random>
#include <iostream>
int main(int argc, char** argv){
  if (argc<3){ std::cerr<<"Usage: nwx_importance --dataset <csv> --model <bundle|bin>\n"; return 2; }
  std::string dataset, model; for(int i=1;i<argc;++i){ std::string a=argv[i]; if(a=="--dataset"&&i+1<argc) dataset=argv[++i]; else if(a=="--model"&&i+1<argc) model=argv[++i]; }
  auto rows = nwx::read_csv_numeric(dataset, ','); if (rows.empty()){ std::cerr<<"empty dataset\n"; return 3; }
  int in_dim=(int)rows[0].size()-1, out_classes=0; nwx::Tensor2D X((int)rows.size(), in_dim); std::vector<int> y;
  for (int i=0;i<(int)rows.size();++i){ for (int j=0;j<in_dim;++j) X(i,j)=rows[i][j]; int lab=(int)rows[i][in_dim]; y.push_back(lab); if(lab+1>out_classes) out_classes=lab+1; }
  nwx::ModelBundle b; if (!nwx::load_bundle(b, model)){ if (!nwx::load_model(b.model, model)) { std::cerr<<"load failed\n"; return 4; } b.has_scaler=false; b.model.temperature=1.0; }
  nwx::Tensor2D X0 = X; if (b.has_scaler) X0 = b.scaler.transform(X0);
  auto base = nwx::softmax(b.model.forward(X0)); double base_acc = nwx::accuracy(base, y);
  std::vector<double> drops(in_dim, 0.0);
  std::mt19937 rng(7);
  for (int f=0; f<in_dim; ++f){
    nwx::Tensor2D Xp = X0;
    std::vector<int> perm(Xp.rows); std::iota(perm.begin(), perm.end(), 0); std::shuffle(perm.begin(), perm.end(), rng);
    for (int i=0;i<Xp.rows;++i) Xp(i,f) = X0(perm[i], f);
    auto probs = nwx::softmax(b.model.forward(Xp));
    double acc = nwx::accuracy(probs, y);
    drops[f] = base_acc - acc;
    std::cout<<"feature_"<<f<<" drop="<<drops[f]<<"\n";
  }
  std::cout<<"base_acc="<<base_acc<<"\n";
  return 0;
}
