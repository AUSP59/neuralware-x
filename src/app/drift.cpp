// SPDX-License-Identifier: Apache-2.0
#include "nwx/csv.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace nwx;
static double psi(const std::vector<double>& base, const std::vector<double>& cur){
  // add small epsilon to avoid div by zero
  double s=0.0;
  for (size_t i=0;i<base.size();++i){
    double b = std::max(1e-6, base[i]);
    double c = std::max(1e-6, cur[i]);
    s += (c-b) * std::log(c/b);
  }
  return s;
}
int main(int argc, char** argv){
  if (argc<5){ std::cerr<<"Usage: nwx_drift --baseline <csv> --current <csv> [--bins 10]\n"; return 2; }
  std::string basef, curf; int B=10;
  for (int i=1;i<argc;++i){ std::string a=argv[i]; if(a=="--baseline"&&i+1<argc) basef=argv[++i]; else if(a=="--current"&&i+1<argc) curf=argv[++i]; else if(a=="--bins"&&i+1<argc) B=std::stoi(argv[++i]); }
  auto rb = read_csv_numeric(basef, ','); auto rc = read_csv_numeric(curf, ',');
  if (rb.empty() || rc.empty()){ std::cerr<<"empty datasets\n"; return 3; }
  int cols = (int)rb[0].size()-1; // ignore label
  for (int f=0; f<cols; ++f){
    std::vector<double> xb, xc;
    for (auto& r : rb) xb.push_back(r[f]);
    for (auto& r : rc) xc.push_back(r[f]);
    std::vector<double> bb = xb;
    std::sort(bb.begin(), bb.end());
    std::vector<double> cuts; cuts.reserve(B-1);
    for (int i=1;i<B; ++i){ size_t idx = (size_t)std::round(i * (bb.size()-1) / (double)B); cuts.push_back(bb[idx]); }
    auto hist = [&](const std::vector<double>& x){ std::vector<double> h(B,0.0); for (double v : x){ int b=0; while (b<(int)cuts.size() && v>cuts[b]) b++; h[b]+=1.0; } for(double &u : h) u/=x.size(); return h; };
    auto hb = hist(xb), hc = hist(xc);
    double s = psi(hb, hc);
    std::cout<<"feature_"<<f<<" psi="<<s<<"\n";
  }
  return 0;
}
