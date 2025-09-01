// SPDX-License-Identifier: Apache-2.0
#include "nwx/bundle.hpp"
#include "nwx/serialize.hpp"
#include "nwx/csv.hpp"
#include "nwx/tensor.hpp"
#include "nwx/metrics.hpp"
#include <iostream>
#include <vector>

int main(int argc, char** argv){
  if (argc<5){ std::cerr<<"Usage: nwx_confusion --dataset <csv> --model <bundle|bin> --classes <N>\n"; return 2; }
  std::string dataset, model; int C=0;
  for (int i=1;i<argc;++i){ std::string a=argv[i];
    if(a=="--dataset"&&i+1<argc) dataset=argv[++i];
    else if(a=="--model"&&i+1<argc) model=argv[++i];
    else if(a=="--classes"&&i+1<argc) C=std::stoi(argv[++i]);
  }
  if (C<=0){ std::cerr<<"classes must be >0\n"; return 3; }
  auto rows = nwx::read_csv_numeric(dataset, ','); if(rows.empty()){ std::cerr<<"empty dataset\n"; return 4; }
  int in_dim=(int)rows[0].size()-1; nwx::Tensor2D X((int)rows.size(), in_dim); std::vector<int> y;
  for (int i=0;i<(int)rows.size();++i){ for(int j=0;j<in_dim;++j) X(i,j)=rows[i][j]; y.push_back((int)rows[i][in_dim]); }
  nwx::ModelBundle b; if(!nwx::load_bundle(b, model)){ if(!nwx::load_model(b.model, model)){ std::cerr<<"load failed\n"; return 5; } b.has_scaler=false; b.model.temperature=1.0; }
  if (b.has_scaler) X = b.scaler.transform(X);
  auto P = nwx::softmax(b.model.forward(X));
  std::vector<std::vector<int>> cm(C, std::vector<int>(C, 0));
  for (int i=0;i<P.rows;++i){ int best=0; double v=P(i,0); for(int j=1;j<P.cols;++j) if(P(i,j)>v){v=P(i,j); best=j;} cm[y[i]][best]++; }
  for (int i=0;i<C;++i){ for (int j=0;j<C;++j){ if (j) std::cout<<","; std::cout<<cm[i][j]; } std::cout<<"\n"; }
  return 0;
}
