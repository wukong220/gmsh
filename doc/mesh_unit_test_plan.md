# 网格生成单元测试计划 — 模块化、核心函数与测试用例

本文档面向两类读者：网格算法工程师与数值/几何学工程师。目标是把网格生成器（尤其是 Gmsh 的局部网格修改子系统）拆分为清晰模块，为每个模块定义可测试的行为，并给出详细、可实施的单元测试用例（含数值容差与期望结果）。

内容目录
- 简介与目标
- 全流程拆解（模块化）
- 每个模块的责任、接口与不变式（invariants）
- 为模块设计的单元测试（具体用例：输入、预期、容差）
- 针对核心函数的详细单元测试矩阵
- 测试实现建议与运行说明（包含独立测试与链接到 gmsh 库的策略）
- 随机/属性测试与回归测试建议
- 参考现有测试位置与下一步任务

## 简介与目标

目标：
- 把复杂的网格生成/修正流程拆分为可独立验证的模块。
- 为关键拓扑/几何操作（例如 `edgeSwap`, `edgeSplit`, `collapseVertex`, `smoothVertex`，以及构建边/点空腔的函数）提供精确、可复现的单元测试用例。
- 在没有构建完整 gmsh 库的情况下，先用轻量化的独立实现（helpers）进行行为验证；随后在有库的环境下切换到`qmTetrahedron::qm`等真实质量度量执行回归测试。

约定：
- 所有几何坐标与判定使用双精度（`double`）。
- 体积比较采用相对与绝对容差：absTol=1e-12, relTol=1e-9（可根据尺度放宽）。
- 质量度量（若用替代实现）使用体积 / meanEdgeLength^3 的比例；当链接 gmsh 时使用 `qmTetrahedron::qm`。

## 全流程拆解（模块化）

将网格生成/局部修改拆为如下模块：

1. Input / Preprocessing
   - 任务：读取/验证几何/网格输入（.geo/.msh/.vtk），构建顶点索引、元素拓扑、边-面-单元邻接表。
   - 不变式：无孤立顶点；每个四面体引用 4 个有效顶点。

2. Geometry Primitives
   - 任务：点/向量操作（减法、叉乘、点乘）、长度、三角形/四面体体积计算、平均边长、绕轴角排序。
   - 不变式：体积计算对顶点顺序的绝对值保持不变（符号可能反映定向）。

3. Topology Utilities
   - 任务：构建边/点/单元邻接（edge->tets、vertex->tets、face->tets），检测边界/内腔、提取空腔顶点/面/单元列表。
   - 不变式：若两个单元共享面，则该面的三顶点与它们相同。

4. Triangulation / Swap Pattern Enumeration
   - 任务：给定 k 环顶点枚举所有对角划分（triangulations），即组合子问题（Catalan 数规模），并把每个三角形映射到对应的四面体重构。
   - 不变式：所有生成的三角形覆盖环的内部并且顶点索引在 [0,k-1]。

5. Local Topology Operators
   - edgeSwap: 为给定内边构建所有候选重连（基于三角化），计算每个候选产生的受影响单元的质量与体积守恒，选择优化目标（例如最大化最小质量）。
   - edgeSplit: 在边上插入新顶点并重新构建局部单元。
   - faceSwap/faceSplit/collapseVertex: 对应面与点的局部拓扑修改。
   - 不变式：合法重连后网格无重叠单元且体积守恒（近似）。

6. Geometric Relocation / Smoothing
   - smoothVertex（包括 `smoothVertexOptimize` 与 LaplaceSmoothing）：在不破坏合法性的前提下调整顶点位置以改善元素质量。
   - 不变式：局部操作不使任何单元体积变为负；所选优化目标（例如 min-quality）优先级应下降或维持。

7. Quality Evaluation
   - 任务：使用统一的质量度量（可配置：QMTET_GAMMA，ETA 等）对每个候选或每次调整进行评估。
   - 不变式：质量值在可比尺度，且对退化单元返回低值（接近 0）。

8. IO / Integration Tests
   - 任务：把小网格写出或读入以便可视化与回归验证（vtk/msh）。

## 为模块设计的单元测试（高层）

每个模块都应具有如下类型的测试：
- 基本算子测试（几何原语）：点算术、叉乘、体积对称性、平均边长。 
- 拓扑工具测试：给定小网格，验证 adjacency 构建正确（边->单元映射）。
- 枚举器测试：验证 k=3..7 的三角化数量（Catalan 数：1,2,5,14,42）。
- 局部拓扑操作测试：对已知 ring（k=3、4、5 等）进行 edgeSwap 枚举并验证候选数量 / 体积守恒 / 选出正确的最佳候选（对称例子）。
- 平滑/几何重定位：对已知小腔体检查 Laplace 平移或积分优化是否不会破坏合法性并能通常改善 min-quality。
- IO 测试：读写 VTK 并确保读回的数据与写入的一致。

每个测试都应该包含：
- 明确的几何输入（顶点数组/四面体索引），
- 预期的数值或属性（例如“候选数=5”，或“体积相等”），
- 容差与判定逻辑。

## 针对核心函数的详细单元测试矩阵

下面列出对关键函数的逐项测试（每个项包含：目标、输入、预期、数值容差、边界/异常情况）：

A. `tet_volume(a,b,c,d)`（几何基础）
- 目标：验证体积计算正确。
- 测试1（简单）：单位正交四面体： a=(0,0,0), b=(1,0,0), c=(0,1,0), d=(0,0,1) -> vol=1/6。
- 容差：absTol=1e-14。
- 测试2（有序反向）：交换两个顶点，体积符号反转，绝对值相等。
- 测试3（退化）：四点共面，体积==0。

B. `mean_edge_length` 与 `tet_quality`（质量度量代理）
- 目标：验证当节点收缩/拉伸时，质量度量反应一致。
- 测试：对固定体积但伸长的四面体比较质量应下降。

C. `triangulations(k)`（三角化枚举）
- 目标：验证枚举正确性。
- 测试：k=3..7 的计数应与 Catalan 数一致。
- 额外：为 k=6、7 用 canonical ordering 校验每个三角形索引都在 [0,k-1] 且不重复覆盖边界异常。

D. `order_ring_by_angle(ring, v1, v2)`
- 目标：正确对环顶点做极角排序（绕 v1->v2 轴）。
- 测试1（均匀圆环）：结果应该是顺序排列的索引（循环移位允许）；检验相邻角差正负正确。
- 测试2（随机扰动）：微扰后仍能保持正确顺序（数值稳定性）。

E. `buildEdgeCavity(edge)` / `computeNeighboringTetsOfACavity`
- 目标：从一条内边构建环顶点顺序、受影响三角/四面体集合。
- 测试：构造已知的 k=3、4、5 腔（小网格），验证返回 ring 顶点数与实际匹配、并且所有被返回的单元确实含有边的两个端点。

F. `edgeSwap`（候选枚举及选择）
- 目标：枚举候选重连，计算体积与最小质量，遵从可接受体积守恒阈值，返回最佳候选。
- 测试 1（对称环）：环顶点均匀分布使多个候选质量相同，验证返回的是第一个（或按实现策略）并且体积与参考体积差小于 relTol。
- 测试 2（非对称）：制造一个候选在质量上明显更好，验证被选中。
- 容差：体积 relTol=1e-9；质量比较加相对阈值 1e-12。

G. `smoothVertex`（局部平滑）
- 目标：在给定质量度量与边界约束下，判断是否有可行的重新定位并验证改进。
- 测试：对一个存在尖锐/低质量单元的中心顶点进行 Laplacian 重置和优化步，验证 min-quality 提升或维持，且没有负体积单元。

H. `edgeSplit`、`faceSwap`、`collapseVertex`
- 目标：验证拓扑与几何合法性（无重叠、体积守恒）、并根据策略（例如不使质量越差）进行约束。
- 测试：对易解析的小配置进行中点分裂、面翻转与顶点塌缩，检查单元计数与体积守恒。

## 测试数据（具体数值示例）

在文档中列出一些可复制的输入（示例）：

1) 单位四面体（用于体积/质量）：
```
pts:
 0: (0,0,0)
 1: (1,0,0)
 2: (0,1,0)
 3: (0,0,1)
```
四面体: [0,1,2,3]

期望: volume=1/6, quality > 0

2) 典型 k=4 edge cavity（用于 edgeSwap）
- pts:
  A=(0,0,0), B=(1,0,0)
  ring: R0=(0,1,0), R1=(0,0,1), R2=(1,1,0), R3=(1,0,1)
- tets (sharing edge A-B):
  [A,B,R0,R1], [A,B,R1,R2], [A,B,R2,R3], [A,B,R3,R0]

期望：triangulation count=2, each candidate 体积之和与参考体积接近

（更多可在 `tests/` 目录中找到类似的具体输入。）

## 测试实现策略

- 两类测试：
  1) 独立/快速的单元测试（当前 repo 的 `tests/` 子目录）：使用 `helpers.h`（已存在）提供几何/质量代理。这类测试无需链接 gmsh 核心，便于快速迭代。你已拥有多个独立测试（`triangulations_count_test`, `edgeSplit_test`, `smoothVertex_test`, `ring3/4/5_test` 等）。
  2) 链接 gmsh 的回归测试：在 `depend/` 或同次 CMake 构建中提供 `shared`/`lib` 目标，测试把质量计算切换到 `qmTetrahedron::qm` 并比对独立实现的偏差。

- CMake 配置建议：
  - 保持 `tests/CMakeLists.txt` 可同时支持“独立构建” 与 “链接 gmsh” 两种模式（当前已经实现了目标检测与 `find_library` 支持）。
  - 添加明确的选项 `-DGMSH_TEST_USE_REAL_QM=ON/OFF` 来在 CMake 层选择是否使用 gmsh 的 `qmTetrahedron::qm`。

## 随机/属性测试与回归

- 随机扰动测试：从一组基准网格（小网格），对顶点做小幅随机扰动（Gaussian），验证在多次迭代内没有生成负体积单元，且关键性质（候选计数、体积守恒）保持。种子固定以便可复现。
- 属性测试（Property-based）：针对 `edgeSwap` 的性质：
  - 对任意合法腔，重连后的所有四面体体积和应与原体积和接近（给定 tol）。
  - 重连后不会产生重复/共面的重叠单元（拓扑合法性）。

## 运行说明（快速回顾）

- 先运行独立测试（无需 gmsh 库）：
  - 用已给出的 `g++` 命令快速构建 tests 下的可执行文件，或使用 CMake/Ninja 在 Linux/WSL 下运行。
- 若要使用 gmsh 的真实 `qm`：把已编译的 gmsh 安装放到 `depend/` 或使用 `-DCMAKE_PREFIX_PATH`，并配置 `-DGMSH_TEST_USE_REAL_QM=ON`（需要在 `tests/CMakeLists.txt` 中加入对该选项的支持）。

## 现有测试与映射

- 已存在（或我已添加）的文件示例：
  - `tests/generate_swap_patterns.cpp` — 枚举测试
  - `tests/swap_candidates_eval.cpp` — 候选计算演示
  - `tests/triangulations_count_test.cpp` — 新增（Catalan 验证）
  - `tests/edgeSplit_test.cpp` — 新增（边分裂）
  - `tests/edge_swap/smoothVertex_test.cpp` — 新增（平滑）
  - `tests/edge_swap/ring3_test.cpp`, `ring4_test.cpp`, `ring5_test.cpp` — 已有示例

## 下一步建议（优先级）

1. 将 `GMSH_TEST_USE_REAL_QM` 添加到 CMake 并在有 `depend/` 安装时启用，以便在 CI 上对真实 `qmTetrahedron::qm` 做回归测试。
2. 为 edgeSwap/edgeSplit/collapseVertex 编写更多确定性小型基准（网格夹具），并对它们进行逐一回归（记录基线质量与体积）。
3. 编写随机化并行测试以检测竞态/数值稳定性（在 CI 上用较小样本快速运行）。

---

文档结束。若你愿意，我可以把上面第 1、2、3 步的具体 CMake 变更、更多小夹具网格文件、以及若干可自动运行的 Python/脚本（汇总测试结果）一并生成并加入仓库。