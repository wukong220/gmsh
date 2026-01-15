# CMake rules for small standalone tests added under tests/
# These tests are small, header-only or self-contained programs that do not
# require linking the full Gmsh library. They are added when ENABLE_TESTS is ON.

cmake_minimum_required(VERSION 3.3)
project(gmsh_unit_tests NONE)

# List of test sources (relative to tests/)
set(UNIT_TEST_SOURCES
  generate_swap_patterns.cpp
  swap_candidates_eval.cpp
  triangulations_count_test.cpp
  order_ring_test.cpp
  edgeSplit_test.cpp
)

# edge_swap tests
set(EDGE_SWAP_SOURCES
  edge_swap/ring3_test.cpp
  edge_swap/ring4_test.cpp
  edge_swap/ring5_test.cpp
  edge_swap/vtk_ring_test.cpp
  edge_swap/smoothVertex_test.cpp
)

# Core targeted unit tests (skeletons) under tests/core
file(GLOB CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp")
foreach(src ${CORE_SOURCES})
  get_filename_component(name ${src} NAME_WE)
  add_executable(${name} ${src})
  target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/geo ${CMAKE_SOURCE_DIR}/src/mesh)
  if(TARGET shared)
    target_link_libraries(${name} PRIVATE shared)
    target_compile_definitions(${name} PRIVATE GMSH_DLL)
  elseif(TARGET lib)
    target_link_libraries(${name} PRIVATE lib)
  else()
    # try to find installed gmsh in depend/ if needed (headers/libs)
    if(NOT DEFINED GMSH_INSTALL_DIR)
      set(GMSH_INSTALL_DIR "${CMAKE_SOURCE_DIR}/depend" CACHE PATH "Path to installed gmsh (headers/libs)")
    endif()
    find_path(GMSH_INCLUDE_DIR NAMES gmsh.h HINTS ${GMSH_INSTALL_DIR} ${CMAKE_PREFIX_PATH})
    if(GMSH_INCLUDE_DIR)
      target_include_directories(${name} PRIVATE ${GMSH_INCLUDE_DIR})
    endif()
    find_library(GMSH_LIB NAMES gmsh gmsh_static gmshlib libgmsh HINTS ${GMSH_INSTALL_DIR}/lib ${CMAKE_PREFIX_PATH}/lib)
    if(GMSH_LIB)
      target_link_libraries(${name} PRIVATE ${GMSH_LIB})
    endif()
  endif()
  add_test(NAME ${name} COMMAND ${name})
endforeach()

# Create executables for unit tests
foreach(src ${UNIT_TEST_SOURCES})
  get_filename_component(name ${src} NAME_WE)
  add_executable(${name} ${src})
  # If gmsh core library targets exist, link them so tests can use qmTetrahedron
  if(TARGET shared)
    target_link_libraries(${name} PRIVATE shared)
    target_compile_definitions(${name} PRIVATE GMSH_DLL)
  elseif(TARGET lib)
    target_link_libraries(${name} PRIVATE lib)
  else()
    if(NOT DEFINED GMSH_INSTALL_DIR)
      set(GMSH_INSTALL_DIR "${CMAKE_SOURCE_DIR}/depend" CACHE PATH "Path to installed gmsh (headers/libs)")
    endif()
    find_path(GMSH_INCLUDE_DIR NAMES gmsh.h HINTS ${GMSH_INSTALL_DIR} ${CMAKE_PREFIX_PATH})
    if(GMSH_INCLUDE_DIR)
      target_include_directories(${name} PRIVATE ${GMSH_INCLUDE_DIR})
    endif()
    find_library(GMSH_LIB NAMES gmsh gmsh_static gmshlib libgmsh HINTS ${GMSH_INSTALL_DIR}/lib ${CMAKE_PREFIX_PATH}/lib)
    if(GMSH_LIB)
      target_link_libraries(${name} PRIVATE ${GMSH_LIB})
    endif()
  endif()

  # Always add project source include dirs so tests can include internal headers
  target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/geo ${CMAKE_SOURCE_DIR}/src/mesh)

  add_test(NAME ${name} COMMAND ${name})
endforeach()

# Edge-swap helpers: header-only included by tests in edge_swap; compile each
foreach(src ${EDGE_SWAP_SOURCES})
  get_filename_component(name ${src} NAME_WE)
  add_executable(${name} ${src})
  target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  if(TARGET shared)
    target_link_libraries(${name} PRIVATE shared)
    target_compile_definitions(${name} PRIVATE GMSH_DLL)
  elseif(TARGET lib)
    target_link_libraries(${name} PRIVATE lib)
  else()
    if(NOT DEFINED GMSH_INSTALL_DIR)
      set(GMSH_INSTALL_DIR "${CMAKE_SOURCE_DIR}/depend" CACHE PATH "Path to installed gmsh (headers/libs)")
    endif()
    find_path(GMSH_INCLUDE_DIR NAMES gmsh.h HINTS ${GMSH_INSTALL_DIR} ${CMAKE_PREFIX_PATH})
    if(GMSH_INCLUDE_DIR)
      target_include_directories(${name} PRIVATE ${GMSH_INCLUDE_DIR})
    endif()
    find_library(GMSH_LIB NAMES gmsh gmsh_static gmshlib libgmsh HINTS ${GMSH_INSTALL_DIR}/lib ${CMAKE_PREFIX_PATH}/lib)
    if(GMSH_LIB)
      target_link_libraries(${name} PRIVATE ${GMSH_LIB})
    endif()
  endif()

  add_test(NAME ${name} COMMAND ${name})
endforeach()

message(STATUS "Added ${UNIT_TEST_SOURCES} and edge-swap tests to CTest")
