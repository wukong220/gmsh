// 单元测试：质量度量（qmTetrahedron::qm）
// 说明：使用内部类型直接构造四面体并调用 qmTetrahedron::qm

#include <iostream>
#include <cassert>
#include "MVertex.h"
#include "MTetrahedron.h"
#include "qualityMeasures.h"

int main()
{
  // Construct a unit tetrahedron: (0,0,0),(1,0,0),(0,1,0),(0,0,1)
  MVertex *v0 = new MVertex(0.0, 0.0, 0.0);
  MVertex *v1 = new MVertex(1.0, 0.0, 0.0);
  MVertex *v2 = new MVertex(0.0, 1.0, 0.0);
  MVertex *v3 = new MVertex(0.0, 0.0, 1.0);

  MTetrahedron *t = new MTetrahedron(v0, v1, v2, v3);

  double vol = t->getVolume();
  if(!(vol > 0.0)) {
    std::cerr << "FAIL: expected positive volume, got " << vol << "\n";
    return 1;
  }

  double q_eta = qmTetrahedron::qm(t, qmTetrahedron::QMTET_ETA, &vol);
  double q_gamma = qmTetrahedron::qm(t, qmTetrahedron::QMTET_GAMMA, &vol);

  // Basic sanity checks: quality measures should be finite and non-negative
  if(!std::isfinite(q_eta) || q_eta <= 0.0) {
    std::cerr << "FAIL: unexpected q_eta=" << q_eta << "\n";
    return 2;
  }
  if(!std::isfinite(q_gamma) || q_gamma <= 0.0) {
    std::cerr << "FAIL: unexpected q_gamma=" << q_gamma << "\n";
    return 3;
  }

  delete t; // does not delete vertices
  delete v0;
  delete v1;
  delete v2;
  delete v3;

  std::cout << "PASS: qmTetrahedron measures basic checks OK (eta=" << q_eta
            << ", gamma=" << q_gamma << ")\n";
  return 0;
}
