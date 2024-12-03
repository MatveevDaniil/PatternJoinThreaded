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
void sim_search_semi_patterns_ompMap_impl(
  const std::vector<std::string>& strings,
  int cutoff,
  char metric,
  str2int& str2idx,
  int_pair_set& out,
  const ints* strings_subset = nullptr,
  bool include_eye = true,
  const std::string &trim_part = ""
) {
          auto start = std::chrono::high_resolution_clock::now();
  str_int_queue pat_str;
  str2ints pat2str;
  int trim_size = trim_part.size();
  map_patterns_omp<trim_direction>(strings, cutoff, 'S', str2idx, strings_subset, pat_str, trim_part, metric);
          auto end = std::chrono::high_resolution_clock::now();
          std::chrono::duration<double> elapsed_seconds = end - start;
          printf("patterns mapping: %f\n", elapsed_seconds.count());
  distance_k_ptr distance_k = get_distance_k(metric);

  start = std::chrono::high_resolution_clock::now();
  if (trim_direction == TrimDirection::No || trim_direction == TrimDirection::Mid || (trim_direction == TrimDirection::End && metric == 'H')) {
    std::pair<std::string, int> entry;
    while (pat_str.try_dequeue(entry)) {
      int str_idx2 = entry.second;
      ints& indices = pat2str[entry.first];
      if (indices.size() < 1) {
        indices.push_back(str_idx2);
        continue;
      }
      for (auto str_idx1 : indices)
        if (distance_k(strings[str_idx1], strings[str_idx2], cutoff))
          out.insert(str_idx1 > str_idx2 ? std::make_pair(str_idx2, str_idx1) : std::make_pair(str_idx1, str_idx2));
      indices.push_back(str_idx2);
    }
  } else {
    std::pair<std::string, int> entry;
    while (pat_str.try_dequeue(entry)) {
      int str_idx2 = entry.second;
      ints& indices = pat2str[entry.first];
      if (indices.size() < 1) {
        indices.push_back(str_idx2);
        continue;
      }
      std::string str2 = trimString<trim_direction>(strings[str_idx2], trim_size);
      for (auto str_idx1 : indices) {
        std::string str1 = trimString<trim_direction>(strings[str_idx1], trim_size);
        if (distance_k(str1, str2, cutoff)) 
          out.insert(str_idx1 > str_idx2 ? std::make_pair(str_idx2, str_idx1) : std::make_pair(str_idx1, str_idx2));
      }
      indices.push_back(str_idx2);
    }
  }
  if (include_eye)
    for (size_t i = 0; i < strings.size(); i++)
      out.insert({i, i});
            end = std::chrono::high_resolution_clock::now();
            elapsed_seconds = end - start;
            printf("patterns iteration=: %f\n", elapsed_seconds.count());
}

template <TrimDirection trim_direction>
void sim_search_semi_patterns_ompReduce_impl(
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
    #pragma omp parallel for schedule(dynamic, 10)
    for (auto entry = pat2str.begin(); entry != pat2str.end(); entry++) {
      if (entry->second.size() > 1)
        for (auto str_idx1 = entry->second.begin(); str_idx1 != entry->second.end(); ++str_idx1) {
          std::string str1 = strings[*str_idx1];
          for (auto str_idx2 = str_idx1 + 1; str_idx2 != entry->second.end(); ++str_idx2)
            if (distance_k(str1, strings[*str_idx2], cutoff)) {
              if (*str_idx1 > *str_idx2)
                out.insert({*str_idx2, *str_idx1});
              else
                out.insert({*str_idx1, *str_idx2});
            }
        }
    }
  } else {
    #pragma omp parallel for schedule(dynamic, 10)
    for (auto entry = pat2str.begin(); entry != pat2str.end(); entry++) {
      if (entry->second.size() > 1)
        for (auto str_idx1 = entry->second.begin(); str_idx1 != entry->second.end(); ++str_idx1) {
          std::string str1 = trimString<trim_direction>(strings[*str_idx1], trim_size);
          for (auto str_idx2 = str_idx1 + 1; str_idx2 != entry->second.end(); ++str_idx2) {
            std::string str2 = trimString<trim_direction>(strings[*str_idx2], trim_size);
            if (distance_k(str1, str2, cutoff)) {
              if (*str_idx1 > *str_idx2)
                out.insert({*str_idx2, *str_idx1});
              else
                out.insert({*str_idx1, *str_idx2});
            }
          }
        }
    }
  }

  if (include_eye)
    for (size_t i = 0; i < strings.size(); i++)
      out.insert({i, i});
}

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
    for (auto entry = pat2str.begin(); entry != pat2str.end(); entry++)
      if (entry->second.size() > 1)
        for (auto str_idx1 = entry->second.begin(); str_idx1 != entry->second.end(); ++str_idx1) {
          std::string str1 = strings[*str_idx1];
          for (auto str_idx2 = str_idx1 + 1; str_idx2 != entry->second.end(); ++str_idx2)
            if (distance_k(str1, strings[*str_idx2], cutoff)) {
              if (*str_idx1 > *str_idx2)
                out.insert({*str_idx2, *str_idx1});
              else
                out.insert({*str_idx1, *str_idx2});
            }
        }
  } else {
    for (auto entry = pat2str.begin(); entry != pat2str.end(); entry++)
      if (entry->second.size() > 1)
        for (auto str_idx1 = entry->second.begin(); str_idx1 != entry->second.end(); ++str_idx1) {
          std::string str1 = trimString<trim_direction>(strings[*str_idx1], trim_size);
          for (auto str_idx2 = str_idx1 + 1; str_idx2 != entry->second.end(); ++str_idx2) {
            std::string str2 = trimString<trim_direction>(strings[*str_idx2], trim_size);
            if (distance_k(str1, str2, cutoff)) {
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