// vtk_ring_test.cpp - read a simple VTK unstructured grid (if present) and evaluate edge candidates
#include "helpers.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// Minimal ASCII VTK unstructured grid reader for POINTS and CELLS (tets)
bool read_vtk(const std::string &path, std::vector<Vertex> &points, std::vector<std::array<int,4>> &tets){
  std::ifstream in(path);
  if(!in) return false;
  std::string line;
  while(std::getline(in,line)){
    if(line.find("POINTS")!=std::string::npos){
      std::istringstream ss(line); std::string tmp; int n; std::string type; ss>>tmp>>n>>type;
      points.resize(n);
      for(int i=0;i<n;i++){ double x,y,z; in>>x>>y>>z; points[i]={x,y,z}; }
    }
    if(line.find("CELLS")!=std::string::npos){
      std::istringstream ss(line); std::string tmp; int n, size; ss>>tmp>>n>>size;
      for(int i=0;i<n;i++){ int cnt,a,b,c,d; in>>cnt>>a>>b>>c>>d; if(cnt==4) tets.push_back({a,b,c,d}); }
    }
  }
  return true;
}

int main(){
  std::vector<Vertex> pts; std::vector<std::array<int,4>> tets;
  if(!read_vtk("tests/test_mesh.vtk", pts, tets)){
    std::cout<<"No tests/test_mesh.vtk found - skipping VTK test\n";
    return 0;
  }
  std::cout<<"Read "<<pts.size()<<" points and "<<tets.size()<<" tets\n";
  // choose an edge from first tet
  if(tets.empty()) return 1;
  auto first = tets[0];
  int vA = first[0], vB = first[1];
  // collect tets sharing this edge
  std::vector<Vertex> ringVerts;
  for(auto &t: tets){ bool hasA=false, hasB=false; int other=-1; for(int i=0;i<4;i++){ if(t[i]==vA) hasA=true; if(t[i]==vB) hasB=true; } if(hasA && hasB){ for(int i=0;i<4;i++){ if(t[i]!=vA && t[i]!=vB) ringVerts.push_back(pts[t[i]]); } } }
  if(ringVerts.size()<3){ std::cout<<"Edge cavity too small ("<<ringVerts.size()<<") - skipping\n"; return 0; }
  Vertex pA = pts[vA], pB = pts[vB];
  std::vector<double> vols, quals; int best = evaluate_candidates(ringVerts,pA,pB,vols,quals);
  std::cout<<"VTK edge candidates="<<vols.size()<<" best="<<best<<" minQual="<<quals[best]<<"\n";
  return 0;
}
