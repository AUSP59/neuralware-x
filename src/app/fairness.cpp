// SPDX-License-Identifier: Apache-2.0
#include "nwx/bundle.hpp"
#include "nwx/csv.hpp"
#include "nwx/metrics.hpp"
#include "nwx/tensor.hpp"
#include <iostream>
#include <map>

using namespace nwx;
int main(int argc, char** argv){
  if (argc<5){ std::cerr<<"Usage: nwx_fairness --dataset <csv> --model <bundle|bin> --group_col <idx> [--positive_label <int>=1]\n"; return 2; }
  std::string dataset, model; int group_col=-1; int pos_label=1;
  for (int i=1;i<argc;++i){ std::string a=argv[i]; 
    if(a=="--dataset" && i+1<argc) dataset=argv[++i];
    else if(a=="--model" && i+1<argc) model=argv[++i];
    else if(a=="--group_col" && i+1<argc) group_col=std::stoi(argv[++i]);
    else if(a=="--positive_label" && i+1<argc) pos_label=std::stoi(argv[++i]);
  }
  if (group_col<0){ std::cerr<<"group_col required\n"; return 3; }
  auto rows = read_csv_numeric(dataset, ','); if(rows.empty()){ std::cerr<<"empty dataset\n"; return 4; }
  int in_dim=(int)rows[0].size()-1;
  Tensor2D X((int)rows.size(), in_dim); std::vector<int> y; std::vector<int> g;
  int out_classes=0;
  for (int i=0;i<(int)rows.size();++i){
    for (int j=0;j<in_dim;++j) X(i,j)=rows[i][j];
    int lab=(int)rows[i][in_dim]; y.push_back(lab); if(lab+1>out_classes) out_classes=lab+1;
    g.push_back((int)((group_col<in_dim)? rows[i][group_col] : -1));
  }
  ModelBundle b; if(!load_bundle(b, model)){ if(!load_model(b.model, model)){ std::cerr<<"load failed\n"; return 5; } b.has_scaler=false; b.model.temperature=1.0; }
  if (b.has_scaler) X = b.scaler.transform(X);
  auto P = softmax(b.model.forward(X));
  // Predictions
  std::vector<int> yhat(P.rows);
  for (int i=0;i<P.rows;++i){ int best=0; double v=P(i,0); for (int j=1;j<P.cols;++j) if (P(i,j)>v){v=P(i,j); best=j;} yhat[i]=best; }
  // Rates per group
  std::map<int,int> cnt, pos, tp, pos_true;
  for (int i=0;i<P.rows;++i){ cnt[g[i]]++; pos[g[i]] += (yhat[i]==pos_label); pos_true[g[i]] += (y[i]==pos_label); tp[g[i]] += (yhat[i]==pos_label && y[i]==pos_label); }
  double min_rate=1e9, max_rate=0.0; for (auto& kv: cnt){ double rate = (double)pos[kv.first]/kv.second; if(rate<min_rate) min_rate=rate; if(rate>max_rate) max_rate=rate; }
  double disparate_impact = (max_rate>0.0) ? (min_rate / max_rate) : 0.0;
  // Equal opportunity difference (TPR gap)
  double min_tpr=1e9, max_tpr=0.0; for(auto& kv: cnt){ double tpr = pos_true[kv.first]? (double)tp[kv.first]/pos_true[kv.first] : 0.0; if(tpr<min_tpr) min_tpr=tpr; if(tpr>max_tpr) max_tpr=tpr; }
  std::cout<<"disparate_impact="<<disparate_impact<<" equal_opportunity_gap="<<(max_tpr-min_tpr)<<"\n";
  for (auto& kv: cnt){ double rate=(double)pos[kv.first]/kv.second; double tpr = pos_true[kv.first]? (double)tp[kv.first]/pos_true[kv.first] : 0.0; std::cout<<"group "<<kv.first<<": rate="<<rate<<" tpr="<<tpr<<"\n"; }
  return 0;
}
