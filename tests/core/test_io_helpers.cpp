// 单元测试骨架：I/O 与辅助类（MVertex/MTetrahedron/IO helpers）

#include <iostream>

int main(){
#if defined(__has_include)
#if __has_include("gmsh.h")
  std::cout << "INFO: gmsh headers detected; IO/helpers integration test placeholder.\n";
  std::cout << "INFO: Add tests that construct MVertex/MTetrahedron, write/read simple VTK/MSH and compare.\n";
#else
  std::cout << "SKIPPED: gmsh headers not available.\n";
#endif
#else
  std::cout << "SKIPPED: compiler does not support __has_include.\n";
#endif
  return 0;
}
