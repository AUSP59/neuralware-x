// SPDX-License-Identifier: Apache-2.0
#include "nwx/bundle.hpp"
#include "nwx/serialize.hpp"
#include "nwx/tensor.hpp"
#include "nwx/model.hpp"
#include <iostream>
#include <vector>
#include <cmath>

using namespace nwx;

static Tensor2D relu_grad(const Tensor2D& Z){ Tensor2D G(Z.rows, Z.cols); for(int i=0;i<Z.rows;++i) for(int j=0;j<Z.cols;++j) G(i,j)= Z(i,j)>0?1.0:0.0; return G; }
static Tensor2D tanh_grad(const Tensor2D& Z){ Tensor2D G(Z.rows, Z.cols); for(int i=0;i<Z.rows;++i) for(int j=0;j<Z.cols;++j){ double t=std::tanh(Z(i,j)); G(i,j)=1.0-t*t; } return G; }

int main(int argc, char** argv){
  if (argc<5){ std::cerr<<"Usage: nwx_explain --dataset <csv> --model <bundle|bin> --topk <k>=5\n"; return 2; }
  std::string dataset, model; int topk=5;
  for (int i=1;i<argc;++i){ std::string a=argv[i];
    if(a=="--dataset"&&i+1<argc) dataset=argv[++i];
    else if(a=="--model"&&i+1<argc) model=argv[++i];
    else if(a=="--topk"&&i+1<argc) topk=std::stoi(argv[++i]);
  }
  auto rows = read_csv_numeric(dataset, ','); if(rows.empty()){ std::cerr<<"empty dataset\n"; return 3; }
  int in_dim=(int)rows[0].size()-1;
  Tensor2D X((int)rows.size(), in_dim); std::vector<int> y(rows.size());
  for (int i=0;i<(int)rows.size();++i){ for(int j=0;j<in_dim;++j) X(i,j)=rows[i][j]; y[i]=(int)rows[i][in_dim]; }
  ModelBundle b; if(!load_bundle(b, model)){ if(!load_model(b.model, model)){ std::cerr<<"load failed\n"; return 4; } b.has_scaler=false; b.model.temperature=1.0; }
  if (b.has_scaler) X = b.scaler.transform(X);
  // Forward with retained intermediates
  auto Z1 = matmul(X, b.model.W1); for(int i=0;i<Z1.rows;++i) for(int j=0;j<Z1.cols;++j) Z1(i,j)+=b.model.b1[j];
  Tensor2D H = activate(Z1, b.model.activation);
  auto Z2 = matmul(H, b.model.W2); for(int i=0;i<Z2.rows;++i) for(int j=0;j<Z2.cols;++j) Z2(i,j)+=b.model.b2[j];
  auto P = softmax(Z2);
  // Explanations: gradient*input wrt predicted class logit
  for (int i=0;i<X.rows;++i){
    int c=0; double v=P(i,0); for(int j=1;j<P.cols;++j) if(P(i,j)>v){v=P(i,j); c=j;}
    // dZ2 wrt chosen class = 1 at c else 0 (approx logits)
    Tensor2D dZ2(1, Z2.cols); for(int j=0;j<Z2.cols;++j) dZ2(0,j) = (j==c)? 1.0 : 0.0;
    // backprop
    // dH = dZ2 * W2^T
    Tensor2D dH(1, H.cols); for(int k=0;k<H.cols;++k){ double acc=0; for(int j=0;j<dZ2.cols;++j) acc += dZ2(0,j)*b.model.W2(k,j); dH(0,k)=acc; }
    // dZ1 = dH * act'(Z1)
    Tensor2D actg = (b.model.activation=="tanh")? tanh_grad(Z1) : relu_grad(Z1);
    Tensor2D dZ1(1, Z1.cols); for(int j=0;j<Z1.cols;++j) dZ1(0,j) = dH(0,j) * actg(i,j);
    // dX = dZ1 * W1^T
    std::vector<double> g(in_dim,0.0);
    for(int k=0;k<in_dim;++k){ double acc=0; for(int j=0;j<dZ1.cols;++j) acc += dZ1(0,j)*b.model.W1(k,j); g[k]=acc; }
    // gradient * input
    std::vector<std::pair<double,int>> pairs; pairs.reserve(in_dim);
    for (int k=0;k<in_dim;++k) pairs.emplace_back(std::abs(g[k]*X(i,k)), k);
    std::partial_sort(pairs.begin(), pairs.begin()+std::min(topk,(int)pairs.size()), pairs.end(), std::greater<>());
    std::cout<<"row "<<i<<" class "<<c<<": ";
    for (int t=0; t<std::min(topk,(int)pairs.size()); ++t){
      if (t) std::cout<<", ";
      std::cout<<"f"<<pairs[t].second<<"="<<pairs[t].first;
    }
    std::cout<<"\n";
  }
  return 0;
}
