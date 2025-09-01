// SPDX-License-Identifier: Apache-2.0
#include "nwx/bundle.hpp"
#include "nwx/serialize.hpp"
#include "nwx/csv.hpp"
#include "nwx/tensor.hpp"
#include "nwx/metrics.hpp"
#include "nwx/metrics_ex.hpp"
#include "nwx/roc_auc.hpp"
#include "nwx/calibration.hpp"
#include <fstream>
#include <iostream>
int main(int argc, char** argv){
  if (argc<5){ std::cerr<<"Usage: nwx_report --dataset <csv> --model <bundle|bin> --out <md>\n"; return 2; }
  std::string dataset, model, out; for(int i=1;i<argc;++i){ std::string a=argv[i]; if(a=="--dataset"&&i+1<argc) dataset=argv[++i]; else if(a=="--model"&&i+1<argc) model=argv[++i]; else if(a=="--out"&&i+1<argc) out=argv[++i]; }
  auto rows = nwx::read_csv_numeric(dataset, ','); if (rows.empty()){ std::cerr<<"empty dataset\n"; return 3; }
  int in_dim=(int)rows[0].size()-1, out_classes=0; nwx::Tensor2D X((int)rows.size(), in_dim); std::vector<int> y;
  for (int i=0;i<(int)rows.size();++i){ for (int j=0;j<in_dim;++j) X(i,j)=rows[i][j]; int lab=(int)rows[i][in_dim]; y.push_back(lab); if(lab+1>out_classes) out_classes=lab+1; }
  nwx::ModelBundle b; if (!nwx::load_bundle(b, model)){ if (!nwx::load_model(b.model, model)) { std::cerr<<"load failed\n"; return 4; } b.has_scaler=false; b.model.temperature=1.0; }
  if (b.has_scaler) X = b.scaler.transform(X);
  auto L = b.model.forward(X); for(double &u: L.data) u /= b.model.temperature; auto P = nwx::softmax(L);
  double acc = nwx::accuracy(P, y);
  auto f1 = nwx::f1_macro_micro(P, y, out_classes);
  double ece = nwx::expected_calibration_error(P, y, 10);
  double auc = (out_classes==2)? nwx::roc_auc_binary(P, y) : nwx::roc_auc_ovr_macro(P, y, out_classes);
  std::ofstream o(out); o<<"# NEURALWARE-X Report\\n\\n"; o<<"- accuracy: "<<acc<<"\\n"; o<<"- f1_macro: "<<std::get<0>(f1)<<"\\n"; o<<"- f1_micro: "<<std::get<1>(f1)<<"\\n"; o<<"- ece: "<<ece<<"\\n"; if(out_classes==2)o<<"- roc_auc: "<<auc<<"\\n"; else o<<"- roc_auc_ovr: "<<auc<<"\\n"; o.close();
  std::cout<<"wrote "<<out<<"\\n";
  return 0;
}
