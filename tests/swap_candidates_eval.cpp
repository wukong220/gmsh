// swap_candidates_eval.cpp
// For a given ring (k=4) and two endpoints v1,v2, enumerate all candidates
// (triangulations) and compute total volume and min tetra quality for each.

#include <cmath>
#include <iostream>
#include <vector>
#include <array>

struct P { double x,y,z; };

double dot(const P &a, const P &b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
P sub(const P &a, const P &b) { return P{a.x-b.x,a.y-b.y,a.z-b.z}; }
P cross(const P &a, const P &b) { return P{a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x}; }

double tet_volume(const P &a,const P &b,const P &c,const P &d){
  P u = sub(b,a), v = sub(c,a), w = sub(d,a);
  double vol6 = dot(u, cross(v,w));
  return std::fabs(vol6) / 6.0;
}

double mean_edge_length(const std::array<P,4> &v){
  double sum = 0; int cnt=0;
  for(int i=0;i<4;i++) for(int j=i+1;j<4;j++){ double dx=v[i].x-v[j].x, dy=v[i].y-v[j].y, dz=v[i].z-v[j].z; sum += std::sqrt(dx*dx+dy*dy+dz*dz); ++cnt; }
  return sum / cnt;
}

double tet_quality(const P &a,const P &b,const P &c,const P &d){
  double vol = tet_volume(a,b,c,d);
  std::array<P,4> vs = {a,b,c,d};
  double ml = mean_edge_length(vs);
  if(ml <= 0) return 0.0;
  return vol / (ml*ml*ml);
}

using Tri = std::array<int,3>;

// Hardcode BuildSwapPattern4 (triangles/trianguls)
const std::vector<Tri> triangles = {{0,1,2},{2,3,0},{1,2,3},{3,0,1}};
const std::vector<std::vector<int>> trianguls = {{0,1},{2,3}};

int main(){
  // ring coordinates (square in z=0)
  std::vector<P> ring = {{1,0,0},{0,1,0},{-1,0,0},{0,-1,0}}; // p0..p3
  P v1 = {0,0,0.5};
  P v2 = {0,0,-0.5};

  // reference volume: compute using triangulation 0 (candidate 0)
  double refVol = 0.0;
  {
    for(int idx : trianguls[0]){
      Tri t = triangles[idx];
      P a = ring[t[0]], b = ring[t[1]], c = ring[t[2]];
      refVol += tet_volume(a,b,c,v1);
      refVol += tet_volume(c,b,a,v2);
    }
  }

  std::cout << "Reference volume (candidate 0): "<<refVol<<"\n\n";

  for(size_t i=0;i<trianguls.size();++i){
    double volSum = 0.0;
    double minQ = 1e100;
    for(int idx : trianguls[i]){
      Tri t = triangles[idx];
      P a = ring[t[0]], b = ring[t[1]], c = ring[t[2]];
      double vA = tet_volume(a,b,c,v1);
      double vB = tet_volume(c,b,a,v2);
      volSum += vA + vB;
      double qA = tet_quality(a,b,c,v1);
      double qB = tet_quality(c,b,a,v2);
      minQ = std::min(minQ, std::min(qA,qB));
    }
    std::cout << "Candidate "<<i<<": volume="<<volSum<<", minQuality="<<minQ;
    if(std::fabs(volSum - refVol) > 1e-10 * (volSum + refVol)) std::cout << "  <-- volume mismatch";
    std::cout << "\n";
  }
  return 0;
}
