#include "edge_swap/helpers.h"
#include <iostream>
#include <vector>

int main(){
  // Known Catalan numbers for triangulations counts of convex polygon with k vertices:
  // k=3 -> 1, k=4 -> 2, k=5 -> 5, k=6 -> 14, k=7 -> 42
  std::vector<int> expected = {0,0,1,2,5,14,42};
  bool ok = true;
  for(int k=3;k<=7;++k){
    auto tri = triangulations(k);
    int cnt = (int)tri.size();
    std::cout<<"k="<<k<<" triangulations="<<cnt<<" expected="<<expected[k]<<"\n";
    if(cnt != expected[k]) ok = false;
  }
  return ok?0:1;
}
