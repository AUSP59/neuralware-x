// SPDX-License-Identifier: Apache-2.0
#include "nwx/trainer.hpp"
#include "nwx/csv.hpp"
#include "nwx/tensor.hpp"
#include <chrono>
#include <iostream>
int main(int argc, char** argv){
  if (argc<2){ std::cerr<<"Usage: nwx_bench <dataset.csv>\n"; return 2; }
  auto rows = nwx::read_csv_numeric(argv[1], ',');
  if (rows.empty()){ std::cerr<<"empty dataset\n"; return 3; }
  int in_dim=(int)rows[0].size()-1, out_classes=0; nwx::Tensor2D X((int)rows.size(), in_dim); std::vector<int> y;
  for (int i=0;i<(int)rows.size();++i){ for (int j=0;j<in_dim;++j) X(i,j)=rows[i][j]; int lab=(int)rows[i][in_dim]; y.push_back(lab); if(lab+1>out_classes) out_classes=lab+1; }
  std::mt19937 rng(7);
  nwx::MLP m; m.init(in_dim, 16, out_classes, rng);
  nwx::TrainConfig cfg; cfg.epochs=500; cfg.lr=0.05; cfg.batch=16; cfg.optimizer="adam"; cfg.val_split=0.2; cfg.patience=25;
  auto t0 = std::chrono::steady_clock::now();
  nwx::train_xent(m, nwx::Dataset{X,y,out_classes}, cfg);
  auto t1 = std::chrono::steady_clock::now();
  double secs = std::chrono::duration<double>(t1-t0).count();
  auto probs = nwx::softmax(m.forward(X));
  std::cout<<"train_time_s="<<secs<<" final_acc="<<nwx::accuracy(probs, y)<<"\n";
  return 0;
}
