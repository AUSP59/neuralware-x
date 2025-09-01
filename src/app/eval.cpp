// SPDX-License-Identifier: Apache-2.0
#include "nwx/bundle.hpp"
#include "nwx/serialize.hpp"
#include "nwx/csv.hpp"
#include "nwx/tensor.hpp"
#include "nwx/metrics.hpp"
#include "nwx/metrics_ex.hpp"
#include "nwx/roc_auc.hpp"
#include "nwx/calibration.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace nwx;

static void usage(){
  std::cout << "Usage: nwx_eval --dataset <csv> (--model <bundle|bin> | --probs <csv>)\n";
}

int main(int argc, char** argv){
  std::string dataset, model, probs_file;
  for (int i=1;i<argc;++i){
    std::string a=argv[i];
    if(a=="--dataset" && i+1<argc) dataset=argv[++i];
    else if(a=="--model" && i+1<argc) model=argv[++i];
    else if(a=="--probs" && i+1<argc) probs_file=argv[++i];
    else if(a=="--help"){ usage(); return 0; }
  }
  if (dataset.empty() || (model.empty() && probs_file.empty())){ usage(); return 2; }
  auto rows = read_csv_numeric(dataset, ','); if (rows.empty()){ std::cerr<<"empty dataset\n"; return 3; }
  int in_dim=(int)rows[0].size()-1, out_classes=0; Tensor2D X((int)rows.size(), in_dim); std::vector<int> y; y.reserve(rows.size());
  for (int i=0;i<(int)rows.size();++i){ for(int j=0;j<in_dim;++j) X(i,j)=rows[i][j]; int lab=(int)rows[i][in_dim]; y.push_back(lab); if(lab+1>out_classes) out_classes=lab+1; }
  Tensor2D P((int)rows.size(), out_classes);
  if (!probs_file.empty()){
    // assume csv rows with out_classes columns
    auto pr = read_csv_numeric(probs_file, ',');
    if((int)pr.size()!=P.rows || (int)pr[0].size()!=out_classes){ std::cerr<<"probs shape mismatch\n"; return 4; }
    for(int i=0;i<P.rows;++i) for(int j=0;j<out_classes;++j) P(i,j)=pr[i][j];
  } else {
    ModelBundle b;
    if (!load_bundle(b, model)){
      if (!load_model(b.model, model)){ std::cerr<<"failed to load model/bundle\n"; return 5; }
      b.has_scaler=false; b.model.temperature=1.0;
    }
    if (b.has_scaler) X = b.scaler.transform(X);
    auto L = b.model.forward(X);
    for (double &u : L.data) u /= b.model.temperature;
    P = softmax(L);
  }
  double acc = accuracy(P, y);
  auto f1 = f1_macro_micro(P, y, out_classes);
  double ece = expected_calibration_error(P, y, 10);
  std::cout << "accuracy="<<acc<<" f1_macro="<<std::get<0>(f1)<<" f1_micro="<<std::get<1>(f1)<<" ece="<<ece;
  if(out_classes==2){ std::cout<<" roc_auc="<<roc_auc_binary(P, y)<<"\n"; }
  else { std::cout<<" roc_auc_ovr="<<roc_auc_ovr_macro(P, y, out_classes)<<"\n"; }
  return 0;
}
