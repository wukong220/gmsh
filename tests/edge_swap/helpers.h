// helpers.h - lightweight helpers for edge-swap unit tests
#pragma once

#include <array>
#include <cmath>
#include <functional>
#include <vector>
#include <algorithm>

struct Vertex { double x,y,z; };
using Tri = std::array<int,3>;

inline Vertex sub(const Vertex &a,const Vertex &b){return Vertex{a.x-b.x,a.y-b.y,a.z-b.z};}
inline double dot(const Vertex &a,const Vertex &b){return a.x*b.x + a.y*b.y + a.z*b.z;}
inline Vertex cross(const Vertex &a,const Vertex &b){return Vertex{a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};}

inline double tet_volume(const Vertex &a,const Vertex &b,const Vertex &c,const Vertex &d){
  Vertex u = sub(b,a), v = sub(c,a), w = sub(d,a);
  double vol6 = dot(u, cross(v,w));
  return std::fabs(vol6) / 6.0;
}

inline double mean_edge_length(const std::array<Vertex,4> &v){
  double sum = 0; int cnt=0;
  for(int i=0;i<4;i++) for(int j=i+1;j<4;j++){ double dx=v[i].x-v[j].x, dy=v[i].y-v[j].y, dz=v[i].z-v[j].z; sum += std::sqrt(dx*dx+dy*dy+dz*dz); ++cnt; }
  return sum / cnt;
}

inline double tet_quality(const Vertex &a,const Vertex &b,const Vertex &c,const Vertex &d){
  double vol = tet_volume(a,b,c,d);
  std::array<Vertex,4> vs = {a,b,c,d};
  double ml = mean_edge_length(vs);
  if(ml <= 0) return 0.0;
  return vol / (ml*ml*ml);
}

// Generate all triangulations for polygon with k vertices (indices 0..k-1)
using Triangulation = std::vector<Tri>;
inline std::vector<Triangulation> triangulations_rec(int l,int r){
  std::vector<Triangulation> out;
  if(r-l<2){ out.push_back(Triangulation()); return out; }
  for(int m=l+1;m<=r-1;++m){
    auto lefts = triangulations_rec(l,m);
    auto rights = triangulations_rec(m,r);
    for(auto &L:lefts) for(auto &R:rights){ Triangulation comb=L; comb.insert(comb.end(), R.begin(), R.end()); comb.push_back({l,m,r}); out.push_back(comb); }
  }
  return out;
}
inline std::vector<Triangulation> triangulations(int k){ return triangulations_rec(0,k-1); }

// Order ring vertices around the edge defined by v1->v2. Input: coords of ring vertices (unsorted) and endpoints v1,v2.
// Output: indices sorted by angle around axis v2-v1.
inline std::vector<int> order_ring_by_angle(const std::vector<Vertex> &ring, const Vertex &v1, const Vertex &v2){
  Vertex e = sub(v2,v1);
  // choose arbitrary vector not parallel to e
  Vertex ref = (std::fabs(e.x) < 0.9) ? Vertex{1,0,0} : Vertex{0,1,0};
  Vertex u = cross(e, ref);
  double un = std::sqrt(dot(u,u));
  if(un < 1e-12) { // fallback
    u = cross(e, Vertex{0,0,1}); un = std::sqrt(dot(u,u));
  }
  u.x/=un; u.y/=un; u.z/=un;
  Vertex w = cross(e, u);
  double wn = std::sqrt(dot(w,w)); w.x/=wn; w.y/=wn; w.z/=wn;
  std::vector<std::pair<double,int>> ang_idx;
  for(size_t i=0;i<ring.size();++i){ Vertex v = sub(ring[i], v1); double xu = dot(v,u); double yw = dot(v,w); double ang = std::atan2(yw,xu); ang_idx.push_back({ang,(int)i}); }
  std::sort(ang_idx.begin(), ang_idx.end(), [](const std::pair<double,int> &a,const std::pair<double,int> &b){ return a.first < b.first; });
  std::vector<int> idx; for(auto &p:ang_idx) idx.push_back(p.second); return idx;
}

// Evaluate candidates: given ring coords (in sorted order or unsorted), endpoints v1,v2, compute for each triangulation
// the total volume and min tetra quality. Returns index of best candidate (max minQuality) and fills vectors.
inline int evaluate_candidates(const std::vector<Vertex> &ring_unsorted, const Vertex &v1, const Vertex &v2,
                               std::vector<double> &outVolumes, std::vector<double> &outMinQuals){
  int k = (int)ring_unsorted.size();
  auto triangs = triangulations(k);
  // order ring
  std::vector<int> order = order_ring_by_angle(ring_unsorted, v1, v2);
  std::vector<Vertex> ring(k);
  for(int i=0;i<k;++i) ring[i] = ring_unsorted[order[i]];
  outVolumes.clear(); outMinQuals.clear();
  outVolumes.reserve(triangs.size()); outMinQuals.reserve(triangs.size());
  double volumeRef = 0; // optional
  for(auto &T:triangs){ double volSum=0; double minQ=1e100; for(auto &tri: T){ Vertex a = ring[tri[0]]; Vertex b = ring[tri[1]]; Vertex c = ring[tri[2]]; double vA = tet_volume(a,b,c,v1); double vB = tet_volume(c,b,a,v2); volSum += vA + vB; double qA = tet_quality(a,b,c,v1); double qB = tet_quality(c,b,a,v2); minQ = std::min(minQ, std::min(qA,qB)); } outVolumes.push_back(volSum); outMinQuals.push_back(minQ); }
  // choose best
  int best = -1; double bestVal = -1e100; for(size_t i=0;i<outMinQuals.size();++i){ if(outMinQuals[i] > bestVal){ bestVal = outMinQuals[i]; best = (int)i; } }
  return best;
}
