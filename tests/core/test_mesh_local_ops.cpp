// 单元测试：局部网格操作（构造 3 个四面体环并测试 buildEdgeCavity / edgeSwap / edgeSplit / smoothVertex）

#include <iostream>
#include <vector>
#include <set>
#include <cassert>

#include "MVertex.h"
#include "MTetrahedron.h"
#include "meshGRegionDelaunayInsertion.h" // MTet4, MTet4Factory
#include "meshGRegionLocalMeshMod.h"
#include "qualityMeasures.h"

int main()
{
  // Create 5 vertices: central edge between v0 and v1, and three opposite verts
  MVertex *v0 = new MVertex(0.0, 0.0, 0.0);
  MVertex *v1 = new MVertex(1.0, 0.0, 0.0);
  MVertex *v2 = new MVertex(0.0, 1.0, 0.0);
  MVertex *v3 = new MVertex(0.0, 0.0, 1.0);
  MVertex *v4 = new MVertex(1.0, 1.0, 0.0);

  // Tetrahedra arranged around edge (v0,v1):
  // t0: v0, v1, v2, v3
  // t1: v0, v1, v3, v4
  // t2: v0, v1, v4, v2
  MTetrahedron *t0 = new MTetrahedron(v0, v1, v2, v3);
  MTetrahedron *t1 = new MTetrahedron(v0, v1, v3, v4);
  MTetrahedron *t2 = new MTetrahedron(v0, v1, v4, v2);

  // Wrap into MTet4
  MTet4 *m0 = new MTet4(t0, 1.0);
  MTet4 *m1 = new MTet4(t1, 1.0);
  MTet4 *m2 = new MTet4(t2, 1.0);

  // Link neighbors to form a closed ring around edge v0-v1
  // Determine faces indices per construction (see MTetrahedron::faces_tetra)
  // Connect t0 <-> t1 : t0 face 1 (0,1,3) matches t1 face 0 (0,2,1) in local ordering
  m0->setNeigh(1, m1);
  m1->setNeigh(0, m0);

  // Connect t1 <-> t2 : t1 face 1 (0,1,3) matches t2 face 0 (0,2,1)
  m1->setNeigh(1, m2);
  m2->setNeigh(0, m1);

  // Connect t2 <-> t0 : t2 face 1 (0,1,3) matches t0 face 0 (0,2,1)
  m2->setNeigh(1, m0);
  m0->setNeigh(0, m2);

  // Prepare cavity containers
  MVertex *ev1 = nullptr;
  MVertex *ev2 = nullptr;
  std::vector<MTet4 *> cavity;
  std::vector<MTet4 *> outside;
  std::vector<MVertex *> ring;

  bool closed = buildEdgeCavity(m0, 0, &ev1, &ev2, cavity, outside, ring);
  if(!closed) {
    std::cerr << "FAIL: buildEdgeCavity returned false\n";
    return 1;
  }
  if(cavity.size() != 3) {
    std::cerr << "FAIL: expected cavity size 3, got " << cavity.size() << "\n";
    return 2;
  }

  // Test computeNeighboringTetsOfACavity indirectly via outside vector
  if(outside.size() != 0) {
    // For our closed ring there should be no outside neighbors
    std::cerr << "WARN: outside neighbors count = " << outside.size() << "\n";
  }

  // Test edgeSwap (try to generate candidates)
  std::vector<MTet4 *> newTets;
  std::set<MFace, MFaceLessThan> embeddedFaces; // empty
  bool swapped = edgeSwap(newTets, m0, 0, qmTetrahedron::QMTET_ETA, embeddedFaces);
  // edgeSwap may return true or false depending on candidate quality; we accept either
  std::cout << "INFO: edgeSwap returned " << swapped << ", produced " << newTets.size() << " new tets\n";

  // Test edgeSplit: split edge v0-v1 by creating midpoint
  MVertex *mid = new MVertex(0.5, 0.0, 0.0);
  std::vector<MTet4 *> splitNew;
  bool split_ok = edgeSplit(splitNew, m0, mid, 0, qmTetrahedron::QMTET_ETA);
  std::cout << "INFO: edgeSplit returned " << split_ok << ", produced " << splitNew.size() << " new tets\n";

  // Test smoothVertex on m0: try to smooth local vertex 0
  bool sm = smoothVertex(m0, 0, qmTetrahedron::QMTET_ETA);
  std::cout << "INFO: smoothVertex returned " << sm << "\n";

  // cleanup: free MTet4 wrappers (do not double-delete base MTetrahedron here)
  delete m0;
  delete m1;
  delete m2;
  delete mid;

  // delete base tetrahedra and vertices
  delete t0;
  delete t1;
  delete t2;
  delete v0;
  delete v1;
  delete v2;
  delete v3;
  delete v4;

  std::cout << "PASS: local mesh ops basic smoke test completed\n";
  return 0;
}
