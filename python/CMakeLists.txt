cmake_minimum_required(VERSION 3.18)
project(patternjoin_python LANGUAGES CXX)

find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG REQUIRED)
if(APPLE)
  find_path(OpenMP_CXX_INCLUDE_DIR omp.h HINTS
    /opt/homebrew/opt/libomp/include /usr/local/opt/libomp/include)
  find_library(OpenMP_omp_LIBRARY omp HINTS
    /opt/homebrew/opt/libomp/lib /usr/local/opt/libomp/lib)
  set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
  set(OpenMP_CXX_LIB_NAMES "omp")
endif()
find_package(OpenMP REQUIRED)
find_package(TBB CONFIG REQUIRED)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")
pybind11_add_module(_core bindings.cpp
  "${SRC_DIR}/sim_search_semi_patterns.cpp"
  "${SRC_DIR}/patterns_generators.cpp"
  "${SRC_DIR}/file_io.cpp")
target_include_directories(_core PRIVATE "${SRC_DIR}")
target_compile_features(_core PRIVATE cxx_std_20)
target_link_libraries(_core PRIVATE OpenMP::OpenMP_CXX TBB::tbb)
set_target_properties(_core PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS _core LIBRARY DESTINATION patternjoin)
