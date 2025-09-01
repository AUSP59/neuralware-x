// SPDX-License-Identifier: Apache-2.0
#include "nwx/csv.hpp"
#include "nwx/trainer.hpp"
#include "nwx/tensor.hpp"
#include "nwx/metrics.hpp"
#include "nwx/metrics_ex.hpp"
#include "nwx/roc_auc.hpp"
#include "nwx/calibration.hpp"
#include <random>
#include <iostream>
int main(int argc, char** argv){
  if (argc<3){ std::cerr<<"Usage: nwx_cv --dataset <csv> --folds K\n"; return 2; }
  std::string dataset; int K=5;
  for (int i=1;i<argc;++i){ std::string a=argv[i];
    if(a=="--dataset" && i+1<argc) dataset=argv[++i];
    else if(a=="--folds" && i+1<argc) K=std::stoi(argv[++i]);
  }
  auto rows = nwx::read_csv_numeric(dataset, ','); if (rows.empty()){ std::cerr<<"empty dataset\n"; return 3; }
  int in_dim=(int)rows[0].size()-1, out_classes=0;
  nwx::Tensor2D X((int)rows.size(), in_dim); std::vector<int> y;
  for (int i=0;i<(int)rows.size();++i){ for(int j=0;j<in_dim;++j) X(i,j)=rows[i][j]; int lab=(int)rows[i][in_dim]; y.push_back(lab); if(lab+1>out_classes) out_classes=lab+1; }
  std::vector<int> idx(X.rows); std::iota(idx.begin(), idx.end(), 0);
  std::mt19937 rng(7); std::shuffle(idx.begin(), idx.end(), rng);
  std::vector<double> accs; accs.reserve(K);
  for (int f=0; f<K; ++f){
    int start = f*X.rows/K; int end = (f+1)*X.rows/K;
    nwx::Tensor2D Xtr(X.rows-(end-start), in_dim), Xte(end-start, in_dim);
    std::vector<int> ytr, yte; ytr.reserve(Xtr.rows); yte.reserve(Xte.rows);
    int it=0, ie=0;
    for (int i=0;i<X.rows;++i){
      if (i>=start && i<end){ for (int j=0;j<in_dim;++j) Xte(ie,j)=X(idx[i],j); yte.push_back(y[idx[i]]); ie++; }
      else { for (int j=0;j<in_dim;++j) Xtr(it,j)=X(idx[i],j); ytr.push_back(y[idx[i]]); it++; }
    }
    std::mt19937 r2(7+f);
    nwx::MLP m; m.init(in_dim, 16, out_classes, r2);
    nwx::TrainConfig cfg; cfg.epochs=600; cfg.optimizer="adam"; cfg.val_split=0.2; cfg.patience=20; cfg.batch=16;
    nwx::train_xent(m, nwx::Dataset{Xtr,ytr,out_classes}, cfg);
    auto probs = nwx::softmax(m.forward(Xte));
    accs.push_back(nwx::accuracy(probs, yte));
    std::cout<<"fold "<<f+1<<"/"<<K<<" acc="<<accs.back()<<"\n";
  }
  double mean=0.0; for(double a: accs) mean+=a; mean/=K;
  double var=0.0; for(double a: accs) var += (a-mean)*(a-mean); var/=K; double std = std::sqrt(var);
  std::cout<<"cv_mean_acc="<<mean<<" cv_std="<<std<<"\n";
  return 0;
}
