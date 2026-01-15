#include "helpers.h"
#include <iostream>
#include <vector>

// Simple standalone test: create a small cavity around a central vertex and
// perform one Laplacian-like relocation (move to average of neighbor positions).
// Then check whether the minimum tetra quality improves.

int main(){
  // central vertex v0 and ring vertices forming 4 surrounding tets
  Vertex v0 = {0.0,0.0,0.0};
  Vertex v1 = {1.0,0.0,0.0};
  Vertex v2 = {0.0,1.0,0.0};
  Vertex v3 = {0.0,0.0,1.0};
  Vertex v4 = {-0.5,0.2,0.1};
  std::vector<std::array<Vertex,4>> tets;
  tets.push_back({v0,v1,v2,v3});
  tets.push_back({v0,v2,v3,v4});
  tets.push_back({v0,v3,v4,v1});
  tets.push_back({v0,v4,v1,v2});

  auto compute_min_quality = [&](const Vertex &center)->double{
    double minQ = 1e100;
    for(auto &t: tets){ std::array<Vertex,4> vs = t; // replace v0 with center
      vs[0] = center; double q = tet_quality(vs[0],vs[1],vs[2],vs[3]); if(q < minQ) minQ=q; }
    return minQ;
  };

  double before = compute_min_quality(v0);
  // Laplacian relocate: average of neighboring vertex positions
  std::vector<Vertex> neigh = {v1,v2,v3,v4};
  Vertex avg={0,0,0}; for(auto &p:neigh){ avg.x+=p.x; avg.y+=p.y; avg.z+=p.z; } avg.x/=neigh.size(); avg.y/=neigh.size(); avg.z/=neigh.size();
  double after = compute_min_quality(avg);
  std::cout<<"smoothVertex_test beforeMinQ="<<before<<" afterMinQ="<<after<<"\n";
  if(after >= before) return 0; // improvement or equal considered success
  return 1; // failure if quality worsened
}
