#include <omp.h>
#include <vector>
#include <unordered_set>
#include "../thirdparty/unordered_dense.h"
#include "../thirdparty/gtl/phmap.hpp"
#include "../thirdparty/concurrentqueue.h"
#include "../src/bounded_edit_distance.hpp"
#include "../thirdparty/small_vector.hpp"
#include "timing_impl.hpp"
#include <tbb/concurrent_vector.h>

using idx_vector = gch::small_vector<size_t>;
using std_map = std::unordered_map<std::string, idx_vector>;
using ankerl_map = ankerl::unordered_dense::map<std::string, idx_vector>;
using tbb_vector = tbb::concurrent_vector<size_t>;
using ints_vector_vector = std::vector<std::vector<std::vector<size_t>>>;
using gtl_p_map = gtl::parallel_flat_hash_map_m<std::string, tbb_vector>;
using gtl_p_set_idxpair = gtl::parallel_flat_hash_set_m<std::pair<size_t, size_t>>;
using gtl_p_set_str = gtl::parallel_flat_hash_set_m<std::string>;

void readFile(
  const std::string& file_name,
  std::vector<std::string>& strings
) {
  std::ifstream file(file_name);
  if (!file) throw std::runtime_error("File does not exist");
  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty() && line.back() == '\n') line.pop_back();
    strings.push_back(line);
  }
  file.close();
}

std::vector<std::string> semi2Patterns(
    const std::string& str
) {
  std::vector<std::string> patterns;
  patterns.reserve((str.length()) * (str.length() + 3) / 2);
  std::string pattern;
  for (size_t i = 0; i < str.size(); ++i) {
    pattern = str;
    pattern.erase(i, 1);
    patterns.push_back(pattern);
    for (size_t j = i + 1; j < str.size(); ++j) {
      pattern = str;
      pattern.erase(i, 1);
      pattern.erase(j - 1, 1);
      patterns.push_back(pattern);
    }
  }
  patterns.push_back(str);
  return patterns;
}

template <typename MapType>
int serial_semipattern_search(
  std::vector<std::string> input,
  const std::string& map_name,
  std::ofstream& ofs,
  size_t true_output_size
) {
  MapType map;
  gtl_p_set_idxpair output;
  gtl_p_set_str patterns;
  std::string N = std::to_string(input.size());

  measure_time(ofs, N + "," + map_name + ",insert", [&]() {
    for (size_t i = 0; i < input.size(); ++i) {
      const std::string& str = input[i];
      for (const auto& pattern : semi2Patterns(str))
        map[pattern].push_back(i);
    }
  });

  measure_time(ofs, N + "," + map_name + ",iteration", [&]() {
    for (const auto& [pattern, idxs] : map)
      for (size_t i = 0; i < idxs.size(); ++i)
        for (size_t j = i + 1; j < idxs.size(); ++j)
          if (idxs[i] != idxs[j]) 
            if (edit_distance_k(input[idxs[i]], input[idxs[j]], 2)) {
              if (idxs[i] < idxs[j])
                output.insert({idxs[i], idxs[j]});
              else
                output.insert({idxs[j], idxs[i]});
            }
  });
  size_t output_size = output.size() + input.size();
  if (output_size != true_output_size)
    throw std::runtime_error("calculation error");

  measure_time(ofs, N + "," + map_name + ",map_clear", [&]() {
    map.clear();
  });

  return 0;
}

template <typename MapType>
int phmap_semipattern_search(
  std::vector<std::string> input,
  const std::string& map_name,
  std::ofstream& ofs,
  int P,
  size_t true_output_size
) {
  MapType map;
  gtl_p_set_idxpair output;
  std::string N = std::to_string(input.size());
  std::string P_str = std::to_string(P);

  measure_time(ofs, N + "," + map_name + ",insert," + P_str, [&]() {
    #pragma omp parallel for num_threads(P)
    for (size_t i = 0; i < input.size(); ++i) {
      const std::string& str = input[i];
      for (const auto& pattern : semi2Patterns(str))
        map[pattern].push_back(i);
    }
  });

  std::vector<tbb_vector*> idxs_vec;
  measure_time(ofs, N + "," + map_name + ",iteration," + P_str, [&]() {
    for (auto& [pattern, idxs] : map)
      idxs_vec.push_back(&idxs);
    #pragma omp parallel for schedule(dynamic, 10) num_threads(P)
    for (auto& idxs_ptr : idxs_vec) {
      auto& idxs = *idxs_ptr;
      for (size_t i = 0; i < idxs.size(); ++i)
        for (size_t j = i + 1; j < idxs.size(); ++j)
          if (idxs[i] != idxs[j]) 
            if (edit_distance_k(input[idxs[i]], input[idxs[j]], 2)) {
              if (idxs[i] < idxs[j])
                output.insert({idxs[i], idxs[j]});
              else
                output.insert({idxs[j], idxs[i]});
            }
    }
  });
  size_t output_size = output.size() + input.size();
  if (output_size != true_output_size) {
    std::cout << output_size << std::endl;
    std::ofstream ofs("check_output");
    for (const auto& [i, j] : output) {
      ofs << input[i] << " " << input[j] << std::endl;
    }
    throw std::runtime_error("calculation error");
  }

  measure_time(ofs, N + "," + map_name + ",map_clear," + P_str, [&]() {
    map.clear();
    idxs_vec.clear();
  });

  return 0;
}

template <typename MapType>
int mapreduce_semipattern_search(
  std::vector<std::string> input,
  const std::string& map_name,
  std::ofstream& ofs,
  int P,
  size_t true_output_size
) {
  std::vector<MapType> maps;
  for (int i = 0; i < P; i++) maps.push_back(MapType());
  gtl_p_set_idxpair output;
  std::string N = std::to_string(input.size());
  std::string P_str = std::to_string(P);
  gtl_p_set_str patterns;
  std::vector<std::string> patterns_vector;
  ints_vector_vector threads_idxs(P);

  measure_time(ofs, N + "," + map_name + ",insert," + P_str, [&]() {
    #pragma omp parallel num_threads(P) 
    {
      int tid = omp_get_thread_num();
      auto& map = maps[tid];
      double wtime = omp_get_wtime();
      #pragma omp for nowait
      for (size_t i = 0; i < input.size(); ++i) {
        const std::string& str = input[i];
        for (const auto& pattern : semi2Patterns(str)) {
          patterns.insert(pattern);
          map[pattern].push_back(i);
        }
      }
      wtime = omp_get_wtime() - wtime;
      printf("insert: thread=%d: %f\n", tid, wtime);
    }
    patterns_vector = std::vector<std::string>(patterns.begin(), patterns.end());
  });

  measure_time(ofs, N + "," + map_name + ",iteration," + P_str, [&]() {
    // This stage combines values (vectors) from local maps
    // into a single `united_vector` for each pattern.
    // To achieve a good parallelism we use a dynamic scheduling
    // and we collect P collcections of `united_vector`s.
    // In result we get a structure [collection_1, ..., collection_P]
    // where collection_i is [united_vector_i_1, ..., united_vector_i_Ni]
    // where united_vector_ij is a vector<size_t> corresponding to some pattern.
    // Notice that Ni is different for each collection_i, 
    // but since we are using dynamic scheduling,
    // the sizes of each collection_i should be balances.
    #pragma omp parallel num_threads(P) 
    {
      int tid = omp_get_thread_num();
      auto& thread_idxs = threads_idxs[tid];
      thread_idxs.reserve(patterns_vector.size() / P);
      double wtime = omp_get_wtime();
      #pragma omp for schedule(dynamic, 1) nowait
      for (size_t i = 0; i < patterns_vector.size(); i++) {
        thread_idxs.push_back(std::vector<size_t>());
        auto& united_vector = thread_idxs.back();
        auto& pattern = patterns_vector[i];
        for (auto& map: maps) {
          if (map.find(pattern) == map.end()) 
            continue;
          auto& local_vector = map[pattern];
          united_vector.insert(united_vector.end(), local_vector.begin(), local_vector.end());
        }
      }
      wtime = omp_get_wtime() - wtime;
      printf("iterate 1: thread=%d: %f\n", tid, wtime);
    }

    // Now we parallely itearate over each collection_i
    #pragma omp parallel num_threads(P) 
    {
      int tid = omp_get_thread_num();
      auto& thread_idxs = threads_idxs[tid];
      double wtime = omp_get_wtime();
      for (auto& idxs : thread_idxs) {
        for (size_t i = 0; i < idxs.size(); ++i)
          for (size_t j = i + 1; j < idxs.size(); ++j)
            if (idxs[i] != idxs[j]) 
              if (edit_distance_k(input[idxs[i]], input[idxs[j]], 2)) {
                if (idxs[i] < idxs[j])
                  output.insert({idxs[i], idxs[j]});
                else
                  output.insert({idxs[j], idxs[i]});
              }
      }
      wtime = omp_get_wtime() - wtime;
      printf("iterate 2: thread=%d: %f\n", tid, wtime);
    }
  });
  size_t output_size = output.size() + input.size();
  if (output_size != true_output_size) {
    std::cout << output_size << std::endl;
    std::ofstream ofs("check_output");
    for (const auto& [i, j] : output) {
      ofs << input[i] << " " << input[j] << std::endl;
    }
    throw std::runtime_error("calculation error");
  }

  measure_time(ofs, N + "," + map_name + ",map_clear," + P_str, [&]() {
    #pragma omp parallel num_threads(P) 
    {
      int tid = omp_get_thread_num();
      maps[tid].clear();
      threads_idxs[tid].clear();
    }
    maps.clear();
    threads_idxs.clear();
    patterns.clear();
    patterns_vector.clear();
  });

  return 0;
}


int main() {
  std::vector<std::string> TEST_FILES = {"../test_data/P00245-aa"};
  std::vector<size_t> true_outputs = {14802311};
  std::vector<int> threads = {1};


  //////////////////
  // Serial Tests //
  //////////////////
  std::ofstream ofs("../test_results/serial_results_map.csv");
  ofs << "N,map_impl,operation,time" << std::endl;
  std::cout << "serial test" << std::endl;
  std::cout << "N,set_impl,operation,time" << std::endl;
  for (size_t i = 0; i < TEST_FILES.size(); ++i) {
    std::vector<std::string> strings;
    readFile(TEST_FILES[i], strings);

    serial_semipattern_search<ankerl_map>(strings, "ankerl", ofs, true_outputs[i]);
    // serial_semipattern_search<std_map>(strings, "std", ofs, true_outputs[i]);
    // serial_semipattern_search<gtl_p_map>(strings, "gtl", ofs, true_outputs[i]);
  }
  ofs.close();

  ////////////////////////
  // Parallel Map Tests //
  ////////////////////////
  std::ofstream ofs_p("../test_results/parallel_results_map.csv");
  ofs_p << "N,map_impl,operation,P,time" << std::endl;
  std::cout << "parallel test" << std::endl;
  std::cout << "N,set_impl,operation,P,time" << std::endl;
  for (size_t i = 0; i < TEST_FILES.size(); ++i) {
    std::vector<std::string> strings;
    readFile(TEST_FILES[i], strings);

    for (int P : threads) {
      mapreduce_semipattern_search<ankerl_map>(
        strings, "ankerl_mapreduce", ofs_p, P, true_outputs[i]);
      // phmap_semipattern_search<gtl_p_map>(
      //   strings, "gtl", ofs_p, P, true_outputs[i]);
    }
  }
  ofs_p.close();

  //////////////////////
  // Map Reduce Tests //
  //////////////////////

  return 0;
}