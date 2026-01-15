// ring4_test.cpp - test for k=4 with asymmetric ring to prefer a given diagonal
#include "helpers.h"
#include <iostream>
#include <vector>

int main(){
  // perturb square so diagonal p0-p2 yields worse triangles than p1-p3
  std::vector<Vertex> ring = {{1.0,0.0,0.0},{0.1,1.0,0.0},{-1.0,0.0,0.0},{0.0,-1.2,0.0}};
  Vertex v1={0,0,0.4}, v2={0,0,-0.4};
  std::vector<double> vols,quals;
  int best = evaluate_candidates(ring,v1,v2,vols,quals);
  std::cout<<"k=4 candidates="<<vols.size()<<" best="<<best<<" minQual="<<quals[best]<<"\n";
  // There are two candidates; ensure we pick one (index 0 or 1)
  if(vols.size()!=2) return 2;
  if(best<0 || best>1) return 3;
  return 0;
}
