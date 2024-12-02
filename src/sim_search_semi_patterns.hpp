#ifndef SIM_SEARCH_SEMI_PATTERNS_H
#define SIM_SEARCH_SEMI_PATTERNS_H

#include <vector>
#include <string>
#include "map_patterns.hpp"
#include "hash_containers.hpp"
#include "file_io.hpp"
#include "patterns_generators.hpp"
#include "bounded_edit_distance.hpp"

template <TrimDirection trim_direction>
void sim_search_semi_patterns_impl(
  const std::vector<std::string>& strings,
  int cutoff,
  char metric,
  str2int& str2idx,
  int_pair_set& out,
  const ints* strings_subset = nullptr,
  bool include_eye = true,
  const std::string &trim_part = ""
) {
  str_int_set pat_str;
  str2ints pat2str;
  int trim_size = trim_part.size();
          auto start = std::chrono::high_resolution_clock::now();
  map_patterns<trim_direction>(strings, cutoff, 'S', str2idx, strings_subset, pat_str, trim_part, metric);
  distance_k_ptr distance_k = get_distance_k(metric);
          auto end = std::chrono::high_resolution_clock::now();
          std::chrono::duration<double> elapsed_seconds = end - start;
          printf("map_patterns: %f\n", elapsed_seconds.count());
          start = std::chrono::high_resolution_clock::now();

  if (trim_direction == TrimDirection::No || trim_direction == TrimDirection::Mid || (trim_direction == TrimDirection::End && metric == 'H')) {
    for (auto entry : pat_str) {
      str_idx2 = entry->second;
      ints indeces& = pat2str[entry->first];
      if (indeces.size() < 1) {
        indices.push_back(str_idx2);
        continue;
      }
      for (auto str_idx1 : indeces)
        if (distance_k(strings[str_idx1], strings[str_idx2], cutoff))
          out.insert(str_idx1 > str_idx2 ? {str_idx2, str_idx1} : {str_idx1, str_idx2});
      indices.push_back(str_idx2);
    }
  } else {
    for (auto entry : pat_str) {
      str_idx2 = entry->second;
      ints indeces& = pat2str[entry->first];
      if (indeces.size() < 1) {
        indices.push_back(str_idx2);
        continue;
      }
      std::string str2 =  trimString<trim_direction>(strings[str_idx2], trim_size);
      for (auto str_idx1 : indeces) {
        std::string str1 = trimString<trim_direction>(strings[str_idx1], trim_size);
        if (distance_k(str1, str2, cutoff)) 
          out.insert(str_idx1 > str_idx2 ? {str_idx2, str_idx1} : {str_idx1, str_idx2});
      }
      indices.push_back(str_idx2);
    }
  }
          end = std::chrono::high_resolution_clock::now();
          elapsed_seconds = end - start;
          printf("pat2str iteration time: %f\n", elapsed_seconds.count());
  if (include_eye)
    for (size_t i = 0; i < strings.size(); i++)
      out.insert({i, i});
}

int sim_search_semi_patterns(
  std::string file_name,
  int cutoff,
  char metric,
  bool include_duplicates
);


#endif // SIM_SEARCH_SEMI_PATTERNS_H