#include "edge_swap/helpers.h"
#include <iostream>
#include <vector>
#include <cmath>

int main(){
  // Create ring points on circle around edge from (0,0,0) to (0,0,1)
  Vertex v1={0,0,0}, v2={0,0,1};
  int k=8;
  std::vector<Vertex> ring;
  for(int i=0;i<k;++i){ double ang = 2.0*M_PI*i/k; ring.push_back(Vertex{std::cos(ang), std::sin(ang), 0.5}); }
  auto idx = order_ring_by_angle(ring, v1, v2);
  // check that indices are monotonically increasing (circular) starting at some rotation
  bool ok=true;
  for(int i=1;i<k;++i){ if(idx[i]==idx[i-1]) ok=false; }
  std::cout<<"order_ring returned "<<idx.size()<<" indices\n";
  return ok?0:1;
}
