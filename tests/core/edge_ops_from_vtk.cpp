// 从 VTK 文件读取点和四面体，构造内部网格对象并测试局部操作 API
// 目标：作为完整的集成单元测试，直接使用 gmsh 源内部类型和函数

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <array>
#include <cassert>

#include "MVertex.h"
#include "MTetrahedron.h"
#include "meshGRegionDelaunayInsertion.h" // MTet4, connectTets
#include "meshGRegionLocalMeshMod.h"
#include "qualityMeasures.h"

static bool read_vtk_to_msh(const std::string &path, std::vector<MVertex*> &verts, std::vector<MTetrahedron*> &tets)
{
  std::ifstream in(path);
  if(!in) return false;
  std::string line;
  int npoints = 0;
  while(std::getline(in, line)){
    if(line.find("POINTS")!=std::string::npos){
      std::istringstream ss(line);
      std::string tmp, type; ss >> tmp >> npoints >> type;
      verts.reserve(npoints);
      for(int i=0;i<npoints;i++){
        double x,y,z; in >> x >> y >> z;
        verts.push_back(new MVertex(x,y,z));
      }
    }
    else if(line.find("CELLS")!=std::string::npos){
      std::istringstream ss(line);
      std::string tmp; int ncells, size; ss >> tmp >> ncells >> size;
      for(int i=0;i<ncells;i++){
        int cnt,a,b,c,d; in >> cnt >> a >> b >> c >> d;
        if(cnt==4){
          // create MTetrahedron using vertex pointers
          MTetrahedron *tet = new MTetrahedron(verts[a], verts[b], verts[c], verts[d]);
          tets.push_back(tet);
        } else {
          // skip non-tet cells
          for(int k=0;k<cnt;k++){ int tmpi; in>>tmpi; }
        }
      }
    }
  }
  return !tets.empty();
}

int main()
{
  std::vector<MVertex*> verts;
  std::vector<MTetrahedron*> tets;
  if(!read_vtk_to_msh("tests/test_mesh.vtk", verts, tets)){
    std::cout << "SKIPPED: tests/test_mesh.vtk missing or contains no tets\n";
    return 0;
  }

  // Wrap tetrahedra into MTet4 objects
  std::vector<MTet4*> mtets;
  mtets.reserve(tets.size());
  std::vector<double> dummySizes(verts.size(), 1.0), dummySizesBGM(verts.size(), 1.0);
  MTet4Factory factory;
  for(auto *tet : tets){
    MTet4 *m = factory.Create(tet, dummySizes, dummySizesBGM);
    mtets.push_back(m);
  }

  // Connect neighbors using existing helper
  connectTets(mtets);

  // Select first tet and its first local edge (edge 0 -> vertices 0-1)
  MTet4 *seed = mtets[0];
  int localEdge = 0;
  MVertex *v1=nullptr, *v2=nullptr;
  std::vector<MTet4*> cavity; std::vector<MTet4*> outside; std::vector<MVertex*> ring;

  bool closed = buildEdgeCavity(seed, localEdge, &v1, &v2, cavity, outside, ring);
  if(!closed){
    std::cerr << "FAIL: buildEdgeCavity reported open cavity\n";
    return 1;
  }
  if(cavity.empty()){
    std::cerr << "FAIL: empty cavity\n";
    return 2;
  }

  std::cout << "INFO: cavity size="<<cavity.size()<<" ring size="<<ring.size()<<"\n";

  // Evaluate quality of existing tets
  double minQ = 1e300; double vol;
  for(auto *m : cavity){
    MTetrahedron *t = m->tet();
    double q = qmTetrahedron::qm(t, qmTetrahedron::QMTET_ETA, &vol);
    if(q < minQ) minQ = q;
  }
  std::cout << "INFO: min quality in cavity (eta) = "<<minQ<<"\n";

  // Try edge swap candidates
  std::vector<MTet4*> newTets;
  std::set<MFace, MFaceLessThan> embeddedFaces;
  bool swapped = edgeSwap(newTets, seed, localEdge, qmTetrahedron::QMTET_ETA, embeddedFaces);
  std::cout << "INFO: edgeSwap returned="<<swapped<<" produced="<<newTets.size()<<"\n";

  // Try edge split
  MVertex *mid = new MVertex( (v1->x()+v2->x())*0.5, (v1->y()+v2->y())*0.5, (v1->z()+v2->z())*0.5 );
  std::vector<MTet4*> splitTets;
  bool split_ok = edgeSplit(splitTets, seed, mid, localEdge, qmTetrahedron::QMTET_ETA);
  std::cout << "INFO: edgeSplit returned="<<split_ok<<" produced="<<splitTets.size()<<"\n";

  // Try smoothVertex on seed
  bool sm = smoothVertex(seed, 0, qmTetrahedron::QMTET_ETA);
  std::cout << "INFO: smoothVertex returned="<<sm<<"\n";

  // Cleanup
  for(auto *m: mtets) delete m;
  for(auto *p: verts) delete p;
  for(auto *p: newTets) delete p;
  for(auto *p: splitTets) delete p;
  delete mid;

  std::cout << "PASS: edge_ops_from_vtk smoke test completed\n";
  return 0;
}
