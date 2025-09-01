// SPDX-License-Identifier: Apache-2.0
#include "nwx/metrics_ex.hpp"
#include <algorithm>

namespace nwx {

std::tuple<double,double> f1_macro_micro(const Tensor2D& probs, const std::vector<int>& labels, int n_classes) {
  std::vector<int> tp(n_classes,0), fp(n_classes,0), fn(n_classes,0);
  for (int i=0;i<probs.rows;++i) {
    int pred=0; double bv=probs(i,0);
    for (int j=1;j<probs.cols;++j) if(probs(i,j)>bv){bv=probs(i,j); pred=j;}
    int y = labels[(size_t)i];
    if (pred == y) tp[y]++; else { fp[pred]++; fn[y]++; }
  }
  auto f1_for = [&](int c){ double P=tp[c]+fp[c]? (double)tp[c]/(tp[c]+fp[c]) : 0.0; double R=tp[c]+fn[c]? (double)tp[c]/(tp[c]+fn[c]) : 0.0; return (P+R)? 2*P*R/(P+R) : 0.0; };
  double macro=0.0; for(int c=0;c<n_classes;++c) macro += f1_for(c); macro/=n_classes;
  long long TP=0,FP=0,FN=0; for(int c=0;c<n_classes;++c){TP+=tp[c];FP+=fp[c];FN+=fn[c];}
  double P=TP+FP? (double)TP/(TP+FP):0.0; double R=TP+FN? (double)TP/(TP+FN):0.0; double micro = (P+R)? 2*P*R/(P+R):0.0;
  return {macro, micro};
}

double pr_auc_binary(const Tensor2D& probs, const std::vector<int>& labels) {
  std::vector<std::pair<double,int>> s(probs.rows);
  for (int i=0;i<probs.rows;++i) s[i] = {probs(i,1), labels[(size_t)i]};
  std::sort(s.begin(), s.end(), [](auto&a,auto&b){return a.first>b.first;});
  double tp=0, fp=0, P=0, N=0;
  for (auto& p: s) (p.second==1?P:N)++;
  double prev_r=0.0, auc=0.0;
  for (auto& p: s) {
    if (p.second==1) tp++; else fp++;
    double prec = (tp+fp)? tp/(tp+fp):1.0;
    double rec = P? tp/P:0.0;
    auc += prec * (rec - prev_r);
    prev_r = rec;
  }
  return auc;
}

} // namespace nwx
