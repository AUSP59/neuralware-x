// SPDX-License-Identifier: Apache-2.0
#include "nwx/csv.hpp"
#include <iostream>
#include <limits>
int main(int argc, char** argv){
  if (argc<2){ std::cerr<<"Usage: nwx_check <dataset.csv>\n"; return 2; }
  auto rows = nwx::read_csv_numeric(argv[1], ',');
  if (rows.empty()){ std::cerr<<"empty dataset\n"; return 3; }
  int cols = (int)rows[0].size();
  for (size_t i=0;i<rows.size();++i){
    if ((int)rows[i].size()!=cols){ std::cerr<<"row "<<i<<" has "<<rows[i].size()<<" cols (expected "<<cols<<")\n"; return 4; }
    for (double v: rows[i]) if (!std::isfinite(v)){ std::cerr<<"non-finite value at row "<<i<<"\n"; return 5; }
  }
  std::cout<<"OK: "<<rows.size()<<" rows, "<<cols<<" columns\n";
  return 0;
}
