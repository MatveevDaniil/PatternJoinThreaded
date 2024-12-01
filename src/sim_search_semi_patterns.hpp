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
  str2ints pat2str;
  int trim_size = trim_part.size();
  map_patterns<trim_direction>(strings, cutoff, 'S', str2idx, strings_subset, pat2str, trim_part, metric);
  distance_k_ptr distance_k = get_distance_k(metric);

  if (trim_direction == TrimDirection::No || trim_direction == TrimDirection::Mid || (trim_direction == TrimDirection::End && metric == 'H')) {
    #pragma omp for collapse(3)
    for (auto entry = pat2str.begin(); entry != pat2str.end(); entry++)
      for (auto str_idx1 = entry->second.begin(); str_idx1 != entry->second.end(); ++str_idx1) {
        for (auto str_idx2 = str_idx1 + 1; str_idx2 != entry->second.end(); ++str_idx2)
          if (distance_k(strings[*str_idx1], strings[*str_idx2], cutoff)) {
            if (*str_idx1 > *str_idx2)
              out.insert({*str_idx2, *str_idx1});
            else
              out.insert({*str_idx1, *str_idx2});
          }
      }
  } else {
    std::vector<std::string> trimmed_strings(strings.size());
    for (size_t i = 0; i < strings.size(); i++)
      trimmed_strings[i] = trimString<trim_direction>(strings[i], trim_size);
    #pragma omp for collapse(3)
    for (auto entry = pat2str.begin(); entry != pat2str.end(); entry++)
      for (auto str_idx1 = entry->second.begin(); str_idx1 != entry->second.end(); ++str_idx1) {
        for (auto str_idx2 = str_idx1 + 1; str_idx2 != entry->second.end(); ++str_idx2) {
          if (distance_k(trimmed_strings[*str_idx1], trimmed_strings[*str_idx2], cutoff)) {
            if (*str_idx1 > *str_idx2)
              out.insert({*str_idx2, *str_idx1});
            else
              out.insert({*str_idx1, *str_idx2});
          }
        }
      }
  }

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