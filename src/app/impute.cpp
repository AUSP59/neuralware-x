// SPDX-License-Identifier: Apache-2.0
#include "nwx/csv.hpp"
#include <iostream>
#include <fstream>
#include <limits>
#include <algorithm>

int main(int argc, char** argv){
  if (argc<5){ std::cerr<<"Usage: nwx_impute --in <csv> --out <csv> [--strategy mean|median|most_frequent=mean]\n"; return 2; }
  std::string in, out, strat="mean";
  for (int i=1;i<argc;++i){ std::string a=argv[i];
    if(a=="--in" && i+1<argc) in=argv[++i];
    else if(a=="--out" && i+1<argc) out=argv[++i];
    else if(a=="--strategy" && i+1<argc) strat=argv[++i];
  }
  auto rows = nwx::read_csv_numeric(in, ','); if (rows.empty()){ std::cerr<<"empty input\n"; return 3; }
  int cols = (int)rows[0].size();
  std::vector<double> fill(cols, 0.0);
  for (int c=0;c<cols;++c){
    std::vector<double> vals;
    for (auto& r: rows){ double v=r[c]; if (std::isfinite(v)) vals.push_back(v); }
    if (vals.empty()) continue;
    if (strat=="median"){ std::sort(vals.begin(), vals.end()); fill[c]=vals[vals.size()/2]; }
    else if (strat=="most_frequent"){ std::sort(vals.begin(), vals.end()); double best=vals[0]; int bc=1, cc=1; for(size_t i=1;i<vals.size();++i){ if(vals[i]==vals[i-1]) cc++; else { if(cc>bc){bc=cc; best=vals[i-1];} cc=1; } } if(cc>bc) best=vals.back(); fill[c]=best; }
    else { double s=0.0; for(double v: vals)s+=v; fill[c]=s/vals.size(); }
  }
  std::ofstream o(out);
  for (auto& r: rows){
    for (int c=0;c<cols;++c){ double v=r[c]; if(!std::isfinite(v)) v=fill[c]; o<<v; if(c+1<cols) o<<","; }
    o<<"\n";
  }
  o.close();
  std::cout<<"wrote "<<out<<"\n";
  return 0;
}
