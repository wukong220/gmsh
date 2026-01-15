// ring5_test.cpp - test for k=5
#include "helpers.h"
#include <iostream>
#include <vector>

int main(){
  std::vector<Vertex> ring = {{1.0,0.0,0.0},{0.3,1.0,0.0},{-0.6,0.8,0.0},{-1.0,-0.1,0.0},{0.2,-1.0,0.0}};
  Vertex v1={0,0,0.5}, v2={0,0,-0.5};
  std::vector<double> vols, quals;
  int best = evaluate_candidates(ring,v1,v2,vols,quals);
  std::cout<<"k=5 candidates="<<vols.size()<<" best="<<best<<" minQual="<<quals[best]<<"\n";
  if(vols.size() != 5) return 2;
  if(best < 0 || best >= (int)vols.size()) return 3;
  return 0;
}
