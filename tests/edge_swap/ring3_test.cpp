// ring3_test.cpp - simple test for k=3
#include "helpers.h"
#include <iostream>
#include <vector>

int main(){
  std::vector<Vertex> ring = {{1,0,0},{0,1,0},{-1,0,0}}; // k=3
  Vertex v1 = {0,0,0.6}, v2={0,0,-0.6};
  std::vector<double> vols, quals;
  int best = evaluate_candidates(ring,v1,v2,vols,quals);
  std::cout<<"k=3 candidates="<<vols.size()<<" best="<<best<<" minQual="<<quals[best]<<"\n";
  if(vols.size()!=1) return 1;
  if(best!=0) return 1;
  return 0;
}
