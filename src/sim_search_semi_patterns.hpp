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
void sim_search_semi_patterns_omp_impl(
  const std::vector<std::string>& strings,
  int cutoff,
  char metric,
  str2int& str2idx,
  int_pair_set& out,
  const ints* strings_subset = nullptr,
  bool include_eye = true,
  const std::string &trim_part = ""
) {
  str2ints_parallel pat2str;
  int trim_size = trim_part.size();
  str2ints_collection pat2str_collection;
  strs_parallel patterns_vector;
  int P = omp_get_max_threads();
  map_patterns_omp<trim_direction>(strings, cutoff, 'S', str2idx, strings_subset, pat2str_collection, trim_part, metric);
  #pragma omp parallel
  {
    int tid = omp_get_thread_num();
    auto& map = pat2str_collection[tid];
    for (auto& [pattern, _]: map) {
      bool found = false;
      for (int tid2 = 0; tid2 < tid; tid2++) {
        auto& map2 = pat2str_collection[tid2];
        if (map2.find(pattern) != map2.end()) { found = true; break; }
      }
      if (!found)
        patterns_vector.push_back(pattern);
    }
  }
  distance_k_ptr distance_k = get_distance_k(metric);
  #pragma omp parallel 
  {
  #pragma omp for schedule(dynamic, 10)
  for (size_t i = 0; i < patterns_vector.size(); i++) {
    auto& pattern = patterns_vector[i];
    std::vector<ints*> loc_idxs_vec;
    loc_idxs_vec.reserve(P);
    for (auto& map : pat2str_collection) {
      if (map.find(pattern) == map.end()) 
        continue;
      loc_idxs_vec.push_back(&map[pattern]);
    }
    for (size_t I = 0; I < loc_idxs_vec.size(); I++) {
      auto& idxs = *loc_idxs_vec[I];
      for (size_t i = 0; i < idxs.size(); ++i) {
        std::string str1 = trimString<trim_direction>(strings[idxs[i]], trim_size);
        for (size_t j = i + 1; j < idxs.size(); ++j) {
          std::string str2 = trimString<trim_direction>(strings[idxs[j]], trim_size);
          if (idxs[i] != idxs[j]) 
            if (distance_k(str1, str2, cutoff)) {
              if (idxs[i] < idxs[j])
                out.insert({idxs[i], idxs[j]});
              else
                out.insert({idxs[j], idxs[i]});
            }
        }
      }
      for (size_t J = I + 1; J < loc_idxs_vec.size(); J++) {
        auto& idxs2 = *loc_idxs_vec[J];
        for (size_t i = 0; i < idxs.size(); ++i) {
          std::string str1 = trimString<trim_direction>(strings[idxs[i]], trim_size);
          for (size_t j = 0; j < idxs2.size(); ++j) {
            std::string str2 = trimString<trim_direction>(strings[idxs2[j]], trim_size);
            if (idxs[i] != idxs2[j]) 
              if (distance_k(str1, str2, cutoff)) {
                if (idxs[i] < idxs2[j])
                  out.insert({idxs[i], idxs2[j]});
                else
                  out.insert({idxs2[j], idxs[i]});
              }
          }
        }
      }
    }
  }
  }

  #pragma omp parallel
  {
  int tid = omp_get_thread_num();
  auto& pat2str_local = pat2str_collection[tid];
  pat2str_local.clear();
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