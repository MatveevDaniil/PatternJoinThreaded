#include <omp.h>
#include <unordered_set>
#include "../thirdparty/unordered_dense.h"
#include "../thirdparty/gtl/phmap.hpp"
#define EMH_CACHE_LINE_SIZE 64
#include "../thirdparty/hash_set8.hpp"
#include "timing_impl.hpp"


using int_pair = std::pair<int, int>;
using element_type = int;
using std_vector = std::vector<element_type>;
using std_set = std::unordered_set<element_type>;
using emhash_set = emhash8::HashSet<element_type>;
using ankerl_set = ankerl::unordered_dense::set<element_type>;
using gtl_p_set = gtl::parallel_flat_hash_set_m<element_type>;


std::vector<element_type> generate_input(size_t N) {
  std::vector<element_type> input;
  input.reserve(N);
  for (size_t i = 0; i < N; ++i)
    input.emplace_back(i);
  return input;
}

template <typename SetType>
int run_serial_test(
  const std::vector<element_type>& input, 
  const std::string& set_name,
  std::ofstream& ofs
) {
  SetType set;
  int rabotyaga = 0;
  int N = input.size();
  std::string N_str = std::to_string(N);

  if constexpr (std::is_same<SetType, std_vector>::value) {
    measure_time(ofs, N_str + "," + set_name + ",insert", [&]() {
      for (const auto& element : input)
        set.push_back(element);
    });
  } else {
    measure_time(ofs, N_str + "," + set_name + ",insert", [&]() {
      for (const auto& element : input)
        set.insert(element);
    });
  }

  int expected_sum = 0;
  for (int i = 0; i < N; ++i) expected_sum += i;
  measure_time(ofs, N_str + "," + set_name + ",iterate", [&]() {
    for (const auto& element : set)
      rabotyaga += element;
  });
  if (rabotyaga != expected_sum) { std::cout << "calculation error!\n"; return -1; }

  measure_time(ofs, N_str + "," + set_name + ",clear", [&]() {
    set.clear();
  });

  return 0;
}

template <typename SetType>
int run_parallel_test(
  const std::vector<element_type>& input, 
  const std::string& set_name, 
  int P,
  std::ofstream& ofs
) {
  SetType set;
  int rabotyaga = 0;
  int N = input.size();
  std::string N_str = std::to_string(N);
  std::string P_str = std::to_string(P);
  
  if constexpr (std::is_same<SetType, gtl_p_set>::value) {
    measure_time(ofs, N_str + "," + set_name + ",insert," + P_str, [&]() {
      #pragma omp parallel for num_threads(P)
      for (size_t i = 0; i < input.size(); ++i)
        set.insert(input[i]);
    });
  } else if constexpr (std::is_same<SetType, std_vector>::value) {
    measure_time(ofs, N_str + "," + set_name + ",insert," + P_str, [&]() {
      #pragma omp parallel for num_threads(P)
      for (const auto& element : input)
        #pragma omp critical
        set.push_back(element);
    });
  } else {
    measure_time(ofs, N_str + "," + set_name + ",insert," + P_str, [&]() {
      #pragma omp parallel for num_threads(P)
      for (const auto& element : input) {
        #pragma omp critical
        set.insert(element);
      }
    });
  }

  int expected_sum = 0;
  for (int i = 0; i < N; ++i) expected_sum += i;
  measure_time(ofs, N_str + "," + set_name + ",iterate," + P_str, [&]() {
    for (const auto& element : set)
      rabotyaga += element;
  });
  if (rabotyaga != expected_sum) { std::cout << "calculation error: " << " " << rabotyaga << std::endl; return -1; }

  measure_time(ofs, N_str + "," + set_name + ",delete," + P_str, [&]() {
    set.clear();
  });

  return 0;
}


int main() {
  std::vector<size_t> sizes = {10, 100, 10000, 50000, 100000, 200000};
  std::vector<int> threads = {1, 4, 8, 16};

  //////////////////
  // Serial Tests //
  //////////////////
  std::ofstream ofs("../test_results/serial_results_set.csv");
  ofs << "N,set_impl,operation,time" << std::endl;
  for (size_t N : sizes) {
    auto input = generate_input(N);
    run_serial_test<std_vector>(input, "vector", ofs);
    run_serial_test<std_set>(input, "std", ofs);
    run_serial_test<ankerl_set>(input, "ankerl", ofs);
    run_serial_test<emhash_set>(input, "emhash", ofs);
    run_serial_test<gtl_p_set>(input, "gtl", ofs);
  }
  ofs.close();

  ////////////////////////
  // Parallel Set Tests //
  ////////////////////////
  ofs.open("../test_results/parallel_results_set.csv");
  ofs << "N,set_impl,operation,P,time" << std::endl;
  for (size_t N : sizes) {
    auto input = generate_input(N);
    for (int P : threads) {
      run_parallel_test<std_vector>(input, "vector", P, ofs);
      run_parallel_test<std_set>(input, "std", P, ofs);
      run_parallel_test<emhash_set>(input, "emhash", P, ofs);
      run_parallel_test<ankerl_set>(input, "ankerl", P, ofs);
      run_parallel_test<gtl_p_set>(input, "gtl", P, ofs);
    }
  }
  ofs.close();

  return 0;
}