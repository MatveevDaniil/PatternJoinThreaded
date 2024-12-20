cmake_minimum_required(VERSION 3.10)
project(pattern_join)

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
find_package(TBB REQUIRED)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(OBJ_DIR "${CMAKE_CURRENT_BINARY_DIR}/obj")

file(GLOB SOURCES "${SRC_DIR}/*.cpp")
add_executable(pattern_join ${SOURCES})

target_link_libraries(pattern_join PUBLIC OpenMP::OpenMP_CXX TBB::tbb)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")