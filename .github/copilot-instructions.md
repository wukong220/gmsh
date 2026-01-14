<!-- Short guide for AI coding agents working on Gmsh 4.11.x source tree -->
# Gmsh — Copilot Instructions (concise)

Purpose
- Help an AI coding agent become productive quickly in this repository.

Quick repo map
- **Core C++ modules:** `src/geo/` (geometry & CAD glue), `src/mesh/` (meshing
  algorithms and data structures), `src/numeric/` (linear algebra / numerics),
  `src/parser/` (`.geo` parser), `src/post/` and `src/graphics/`.
- **Bindings & API:** `api/` contains `gmsh.h`, `gmsh.py`, `gmsh.jl`, `gmsh.f90`.
- **Contrib:** `contrib/` holds optional 3rd-party components (Eigen, Netgen,
  QuadMeshingTools, etc.). Treat these as optional integration points.
- **Docs & examples:** `doc/`, `tutorials/`, and `examples/` (use these for
  realistic usage examples and expected behaviors).

Build & test workflows (concrete)
- Primary build system: CMake. Default build type is `RelWithDebInfo`.
- Typical Linux/macOS (bash):
  ```bash
  mkdir build && cd build
  cmake .. -DENABLE_BUILD_DYNAMIC=1 -DENABLE_FLTK=0
  make -j$(nproc)
  ctest -j4         # run tests if -DENABLE_TESTS=ON
  ```
- Windows (MSVC): open `build/gmsh.sln` or use `cmake` to generate a Visual
  Studio solution in `build/`, then build with Visual Studio or `msbuild`.
- Useful CMake flags frequently required in patches/tests:
  - `-DENABLE_BUILD_DYNAMIC=ON` (build shared libs and dynamic executable)
  - `-DENABLE_FLTK=ON/OFF` (GUI dependency)
  - `-DENABLE_OCC=ON` (OpenCASCADE/STP support)
  - `-DENABLE_TESTS=ON` (enable unit/integration tests)
  - `-DCMAKE_INSTALL_PREFIX=/opt/path` (install location)
- Verbose build logs: `make VERBOSE=1` or set `CMAKE_VERBOSE_MAKEFILE=ON`.

Project-specific conventions & patterns
- CMake options are named `ENABLE_<FEATURE>` (see top-level `CMakeLists.txt`).
  When adding features or tests, follow that naming pattern and respect the
  existing `opt()` macro usage.
- Public vs private API: `api/` holds the stable public bindings. The private
  internal headers live under `src/*`. Avoid touching `api/` unless updating
  bindings or API surface intentionally.
- C++ standard: code targets C++11. Keep new additions compatible with that
  level unless a broader modernization is explicitly requested.
- Module responsibilities:
  - `src/geo/`: interpret `.geo` scripts, build geometry models and CAD glue.
  - `src/mesh/`: mesh data structures, element generation and refinement.
  - `src/parser/`: parsing of `.geo` and option files.

Integration points & external deps
- Optional libraries are integrated via CMake (OCC, FLTK, Eigen, METIS, PETSc,
  etc.). When modifying code that relies on an optional library, guard with the
  corresponding `#ifdef`/CMake `ENABLE_` option and update CMakeLists accordingly.
- Language bindings: Python/Fortran/Julia wrappers are in `api/` and controlled
  by CMake `ENABLE_WRAP_*` options; when changing APIs, update `api/gmsh.h` and
  the language wrappers.

Code examples to reference
- To locate mesh algorithm entry points, inspect `src/mesh/` (e.g.
  `src/mesh/meshGRegionLocalMeshMod.cpp`).
- To see how the Python API maps to C++ internals, open `api/gmsh.py` and
  `api/gmsh.h` for the C/C++ declarations.

Testing & debugging tips
- Enable `-DENABLE_TESTS=ON` and run `ctest` from the `build/` directory.
- Use `-DCMAKE_BUILD_TYPE=Debug` or `RelWithDebInfo` when debugging.
- For tracing compilation/runtime problems, prefer `make VERBOSE=1` and
  `ctest -V -R <testname>`.

What to edit / where to add code
- Add new core C++ code under `src/<subsystem>/` and expose APIs in headers
  placed alongside implementation files. Update `CMakeLists.txt` in that
  subsystem or the top-level `CMakeLists.txt` to include new sources.
- For small changes to bindings, update `api/` wrappers and the top-level CMake
  variables that control wrapper generation.

Safety & scope notes for AI agents
- Do not modify licensing files (`LICENSE.txt`, `CREDITS.txt`).
- Avoid large refactors without an explicit developer request — prefer
  targeted, well-tested changes.
- When adding or changing public APIs, include/adjust language bindings and
  a short example in `examples/` or `tutorials/` demonstrating the change.

If something is unclear
- Ask which platform & build flags the maintainer expects (Linux vs Windows,
  static vs dynamic build, GUI vs headless).
- Point to a failing build/test and I'll search the build logs and relevant
  CMake logic to propose a minimal patch.

Key files & directories (quick reference)
- `CMakeLists.txt` — top-level build configuration
- `src/` — core implementation divided by subsystem
- `api/` — stable public APIs and language bindings
- `contrib/` — optional third-party modules
- `doc/`, `tutorials/`, `examples/` — usage and expected behavior

-- End of file
