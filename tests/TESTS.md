Tests in this directory
======================

概览
----
这些测试直接使用 gmsh 源代码中的内部类型（例如 `MVertex`, `MTetrahedron`, `MTet4`）和局部网格修改 API（例如 `buildEdgeCavity`, `edgeSwap`, `edgeSplit`, `smoothVertex`）。它们的目标是作为作者级的单元/集成测试，直接验证核心算法的行为。

主要测试文件
------------
- `core/test_quality_metrics.cpp`：构造单个标准四面体并调用 `qmTetrahedron::qm`，检查返回的质量度量是有限且为正的。
- `core/test_mesh_local_ops.cpp`：手动构造 3 个围绕同一条边的四面体，设置邻接关系（简单手工），并调用 `buildEdgeCavity`、`edgeSwap`、`edgeSplit`、`smoothVertex` 进行烟雾测试。
- `core/edge_ops_from_vtk.cpp`：从 `tests/test_mesh.vtk` 读取点与四面体，构造 `MVertex`/`MTetrahedron`/`MTet4`，调用 `connectTets` 建立邻接，然后调用 `buildEdgeCavity`、`edgeSwap`、`edgeSplit`、`smoothVertex`。如果 `tests/test_mesh.vtk` 不存在，则该测试会优雅地跳过。

如何运行
---------
建议使用一个新的、干净的构建目录（避免覆盖当前 `build/`），例如使用 Ninja：

```bash
mkdir -p build-ninja
cd build-ninja
cmake -G Ninja -DENABLE_TESTS=ON ../
cmake --build . --target tests
ctest -j4 --output-on-failure
```

说明与限制
---------------
- 这些测试直接包含并依赖仓库内的头文件，某些 API（例如 `edgeSwap`、`edgeSplit`）在实现中可能会访问其他内部状态或配置（如 `CTX::instance()`、`GRegion`），在极端情况下测试可能打印警告或返回失败。测试设计为“烟雾测试”并打印信息；必要时可进一步增强断言。
- 如果你希望我把某些测试改为更严格的断言（例如期望具体的交换数或质量阈值），请告诉我需要针对的几何/网格样例，我会据此更新测试并在 `tests/` 下加入对应输入数据文件。

下一步建议
-------------
- 添加更多 VTK 示例（放到 `tests/`），覆盖常见对边、开腔、含边界的情形。
- 将测试结果纳入 CI：配置一个 Linux/Ninja 构建器以避免 Windows/MSVC 的 OpenMP/pragma 问题。
