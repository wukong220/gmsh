#include "edge_swap/helpers.h"
#include <iostream>
#include <vector>

// Simple edge-split style test: create two tets sharing a long edge, split the edge
// by midpoint and replace with four tets; check volume conservation and min quality
// does not drastically worsen.

int main(){
  // endpoints of long edge
  Vertex A={0,0,0}; Vertex B={2,0,0};
  // two other vertices forming two tets sharing edge A-B
  Vertex C={0,1,0}; Vertex D={0,0,1};

  // original tets: (A,B,C,D) and (B,A,D,C) - orientation doesn't matter for volume
  double vol1 = tet_volume(A,B,C,D);
  double vol2 = tet_volume(B,A,D,C);
  double total_before = vol1 + vol2;
  double q1 = tet_quality(A,B,C,D);
  double q2 = tet_quality(B,A,D,C);
  double min_before = std::min(q1,q2);

  // split edge AB at midpoint M
  Vertex M = {(A.x+B.x)/2.0, (A.y+B.y)/2.0, (A.z+B.z)/2.0};
  // create four tets replacing the two: (A,M,C,D), (M,B,C,D) and similarly swapped
  double vA = tet_volume(A,M,C,D);
  double vB = tet_volume(M,B,C,D);
  double total_after = vA + vB; // should equal total_before
  double qa = tet_quality(A,M,C,D);
  double qb = tet_quality(M,B,C,D);
  double min_after = std::min(qa,qb);

  std::cout<<"before vol="<<total_before<<" after vol="<<total_after<<"\n";
  std::cout<<"before minQ="<<min_before<<" after minQ="<<min_after<<"\n";

  if(std::abs(total_before - total_after) > 1e-9) return 2; // volume not conserved
  // allow some tolerance for min quality changes; success if after >= 0.5*before
  if(min_after + 1e-15 < 0.5*min_before) return 1;
  return 0;
}
