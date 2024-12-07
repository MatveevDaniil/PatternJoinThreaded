cmake_minimum_required(VERSION 3.10)
project(hash_structure_performance_testing)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -Wpedantic -Werror")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I/usr/local/opt/libomp/include")
  set(OpenMP_C_LIB_NAMES "omp")
  set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I/usr/local/opt/libomp/include")
  set(OpenMP_CXX_LIB_NAMES "omp")
  set(OpenMP_omp_LIBRARY "/usr/local/opt/libomp/lib/libomp.dylib")
endif()

find_package(OpenMP REQUIRED)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(OBJ_DIR "${CMAKE_CURRENT_BINARY_DIR}/obj")

file(GLOB THREADED_HASH_SETS_SOURCES "${SRC_DIR}/threaded_hash_sets.cpp")
add_executable(threaded_hash_sets ${THREADED_HASH_SETS_SOURCES})
target_link_libraries(threaded_hash_sets PUBLIC OpenMP::OpenMP_CXX)

find_package(TBB REQUIRED)
file(GLOB THREADED_HASH_MAPS_SOURCES "${SRC_DIR}/threaded_hash_maps.cpp")
add_executable(threaded_hash_maps ${THREADED_HASH_MAPS_SOURCES})
target_link_libraries(threaded_hash_maps PUBLIC OpenMP::OpenMP_CXX TBB::tbb)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")