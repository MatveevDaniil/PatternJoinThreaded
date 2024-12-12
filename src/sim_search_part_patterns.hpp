#ifndef SIM_SEARCH_PART_PATTERNS_HPP
#define SIM_SEARCH_PART_PATTERNS_HPP

#include <vector>
#include <string>
#include "map_patterns.hpp"
#include "hash_containers.hpp"
#include "sim_search_semi_patterns.hpp"
#include "sim_search_patterns.hpp"
#include "file_io.hpp"
#include "bounded_edit_distance.hpp"
#include "trim_strings.hpp"
#include "omp.h"
#include <iostream>
#include <chrono>

extern size_t SIM_SEARCH_THRESHOLD;
extern size_t OMP_SIM_SEARCH_THRESHOLD;

template <TrimDirection trim_direction>
inline void check_part(
  std::vector<std::string> &strings,
  int cutoff,
  char metric,
  str2int& str2idx,
  str2ints& part2strings,
  int_pair_set& out
) {
  distance_k_ptr distance_k = get_distance_k(metric);
  
  std::vector<std::pair<std::string, ints>*> entries_small;
  std::vector<std::pair<std::string, ints>*> entries_large;
  for (auto& entry : part2strings)
    if (entry.second.size() < OMP_SIM_SEARCH_THRESHOLD)
      entries_small.push_back(&entry);
    else
      entries_large.push_back(&entry);
  
  // PROCESS SMALL ENTRIES USING EXTENAL THREADING 
  #pragma omp parallel 
  {
  double wtime = omp_get_wtime();
  int thread_id = omp_get_thread_num();
  #pragma omp for schedule(dynamic,1) nowait
  for (size_t i = 0; i < entries_small.size(); ++i) {
    const auto* entry = entries_small[i];
    int part_len = entry->first.size();
    if (entry->second.size() == 1)
      out.insert({entry->second[0], entry->second[0]});
    else if (entry->second.size() < SIM_SEARCH_THRESHOLD) {
      const ints *string_indeces = &(entry->second);
      std::vector<std::string> trimmed_strings(string_indeces->size());
      if (trim_direction == TrimDirection::Mid || (trim_direction == TrimDirection::End && metric == 'H')) {
        if (trim_direction == TrimDirection::Mid) {
          MidTrimFunc midTrim = getMidTrimFunc(metric);
          for (size_t i = 0; i < string_indeces->size(); i++)
            trimmed_strings[i] = midTrim(strings[string_indeces->at(i)], entry->first);
        } else
          for (size_t i = 0; i < string_indeces->size(); i++)
            trimmed_strings[i] = trimString<trim_direction>(strings[string_indeces->at(i)], part_len);
        for (size_t i = 0; i < string_indeces->size(); i++) {
          for (size_t j = i; j < string_indeces->size(); j++) {
            std::string trim_str1 = trimmed_strings[i];
            size_t str_idx1 = string_indeces->at(i);
            if (i == j) {
              out.insert({str_idx1, str_idx1});
              continue;
            }
            std::string str1 = strings[str_idx1];
            std::string trim_str2 = trimmed_strings[j];
            size_t str_idx2 = string_indeces->at(j);
            std::string str2 = strings[str_idx2];
            if (distance_k(trim_str1, trim_str2, cutoff) && distance_k(str1, str2, cutoff)) {
              if (str_idx1 > str_idx2)
                out.insert({str_idx2, str_idx1});
              else
                out.insert({str_idx1, str_idx2});
            }
          }
        }
      } else {
        for (size_t i = 0; i < string_indeces->size(); i++)
          trimmed_strings[i] = trimString<trim_direction>(strings[string_indeces->at(i)], part_len);
        for (size_t i = 0; i < string_indeces->size(); i++) {
          for (size_t j = i; j < string_indeces->size(); j++) {
            std::string trim_str1 = trimmed_strings[i];
            size_t str_idx1 = string_indeces->at(i);
            if (i == j) {
              out.insert({str_idx1, str_idx1});
              continue;
            }
            std::string trim_str2 = trimmed_strings[j];
            size_t str_idx2 = string_indeces->at(j);
            if (distance_k(trim_str1, trim_str2, cutoff)) {
              if (str_idx1 > str_idx2)
                out.insert({str_idx2, str_idx1});
              else
                out.insert({str_idx1, str_idx2});
            }
          }
        }
      } 
    } else {
      sim_search_semi_patterns_impl<trim_direction>(
        strings, cutoff, metric, str2idx, out, &entry->second, false, entry->first);
    }
  }
  wtime = omp_get_wtime() - wtime;
  printf("thread_small=%d: %f\n", thread_id, wtime);
  }

  auto start = std::chrono::high_resolution_clock::now();
  // PROCESS LARGE ENTRIES USING INTERNAL THREADING
  for (size_t i = 0; i < entries_large.size(); i++) {
    const auto* entry = entries_large[i];
          auto start = std::chrono::high_resolution_clock::now();
    std::cout << "out size " << out.size() << std::endl;
    sim_search_semi_patterns_omp_impl<trim_direction>(
      strings, cutoff, metric, str2idx, out, &entry->second, false, entry->first);
    std::cout << "out size " << out.size() << std::endl;
          auto end = std::chrono::high_resolution_clock::now();
          std::chrono::duration<double> elapsed_seconds = end - start;
          printf("large entry size=%ld: %f\n", entry->second.size(), elapsed_seconds.count());
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  printf("large total: %f\n", elapsed_seconds.count());
}

int sim_search_part_patterns(
  std::string file_name,
  int cutoff,
  char metric,
  bool include_duplicates
);


#endif // SIM_SEARCH_PART_PATTERNS_HPP