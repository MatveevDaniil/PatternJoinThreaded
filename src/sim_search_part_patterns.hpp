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
  std::vector<std::pair<std::string, ints>*> entries;
  entries.reserve(part2strings.size());
  for (auto& entry : part2strings)
    entries.push_back(&entry);
  #pragma omp parallel 
  {
  int thread_id = omp_get_thread_num();
  double wtime = omp_get_wtime();
  for (size_t i = 0; i < entries.size(); ++i) {
    const auto* entry = entries[i];
    int part_len = entry->first.size();
    if (entry->second.size() == 1)
      out.insert({entry->second[0], entry->second[0]});
    else if (entry->second.size() < 50) {
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
        #pragma omp for collapse(2) nowait
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
        #pragma omp for collapse(2)
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
      // auto start = std::chrono::high_resolution_clock::now();
      sim_search_semi_patterns_impl<trim_direction>(
        strings, cutoff, metric, str2idx, out, &entry->second, false, entry->first);
      // auto end = std::chrono::high_resolution_clock::now();
      // std::chrono::duration<double> elapsed_seconds = end - start;
      // printf("s=%ld t=%d: %f\n", entry->second.size(), thread_id, elapsed_seconds.count());
    }
  }
  wtime = omp_get_wtime() - wtime;
  printf("thread=%d: %f\n", thread_id, wtime);
  }
}

int sim_search_part_patterns(
  std::string file_name,
  int cutoff,
  char metric,
  bool include_duplicates
);


#endif // SIM_SEARCH_PART_PATTERNS_HPP