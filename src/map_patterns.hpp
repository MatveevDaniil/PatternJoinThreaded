#ifndef MAP_PATTERNS_HPP
#define MAP_PATTERNS_HPP

#include <omp.h>
#include <vector>
#include <string>
#include <iostream>
#include <mutex>
#include "patterns_generators.hpp"
#include "hash_containers.hpp"
#include "trim_strings.hpp"


template <TrimDirection trim_direction>
void map_patterns_omp(
  const std::vector<std::string>& strings,
  int cutoff,
  char pattern_type,
  str2int& str2idx,
  const ints* strings_subset,
  str2ints_parallel& pat2str,
  const std::string& trim_part = "",
  const char metric_type = 'L'
) {
  PatternFuncType PatternFunc = getPatternFunc(cutoff, pattern_type);
  int trim_size = trim_part.size();
  str2ints_collection pat2str_collection;
  strs_parallel patterns;
  int thread_num = omp_get_max_threads();
  for (int i = 0; i < thread_num; i++) {
    pat2str_collection.push_back(str2ints());
  }
  std::cout << "mapping stage started" << std::endl;
  #pragma omp parallel 
  {
  int tid = omp_get_thread_num();
  str2ints& pat2str_local = pat2str_collection[tid];
  if (strings_subset == nullptr) {
    #pragma omp for
    for (std::string str: strings) {
      for (const auto& pattern: PatternFunc(str, nullptr)) {
        patterns.insert(pattern);
        pat2str_local[pattern].push_back(str2idx[str]);
      }
    }
  }
  else {
    if (trim_direction == TrimDirection::No) {
      #pragma omp for
      for (int str_idx: *strings_subset) {
        for (const auto& pattern: PatternFunc(strings[str_idx], nullptr)) {
          patterns.insert(pattern);
          pat2str_local[pattern].push_back(str_idx);
        }
      }
    }
    else if (trim_direction == TrimDirection::Mid) {
      MidTrimFunc midTrim = getMidTrimFunc(metric_type);
      #pragma omp for
      for (int str_idx: *strings_subset) {
        for (const auto& pattern: PatternFunc(midTrim(strings[str_idx], trim_part), nullptr)) {
          patterns.insert(pattern);
          pat2str_local[pattern].push_back(str_idx);
        }
      }
    } else {
      #pragma omp for
      for (int str_idx: *strings_subset) {
        for (const auto& pattern: PatternFunc(trimString<trim_direction>(strings[str_idx], trim_size), nullptr)) {
          patterns.insert(pattern);
          pat2str_local[pattern].push_back(str_idx);
        } 
      }
    }
  }
  }
  std::vector<std::string> patterns_vector(patterns.begin(), patterns.end());

  std::cout << "reduce stage started" << std::endl;
  #pragma omp parallel for
  for (size_t i = 0; i < patterns_vector.size(); i++) {
    std::string pattern = patterns_vector[i];
    auto& vec = pat2str[pattern];
    for (auto& pat2str_local: pat2str_collection) {
      for (int str_idx: pat2str_local[pattern]) {
        vec.push_back(str_idx);
      }
    }
  }
}

template <TrimDirection trim_direction>
void map_patterns(
  const std::vector<std::string>& strings,
  int cutoff,
  char pattern_type,
  str2int& str2idx,
  const ints* strings_subset,
  str2ints& pat2str,
  const std::string& trim_part = "",
  const char metric_type = 'L'
) {
  PatternFuncType PatternFunc = getPatternFunc(cutoff, pattern_type);
  int trim_size = trim_part.size();

  if (strings_subset == nullptr) {
    for (std::string str: strings)
      for (const auto& pattern: PatternFunc(str, nullptr))
        pat2str[pattern].push_back(str2idx[str]);
  }
  else {
    if (trim_direction == TrimDirection::No) {
      for (int str_idx: *strings_subset)
        for (const auto& pattern: PatternFunc(strings[str_idx], nullptr)) 
          pat2str[pattern].push_back(str_idx);
    }
    else if (trim_direction == TrimDirection::Mid) {
      MidTrimFunc midTrim = getMidTrimFunc(metric_type);
      for (int str_idx: *strings_subset)
        for (const auto& pattern: PatternFunc(midTrim(strings[str_idx], trim_part), nullptr)) 
          pat2str[pattern].push_back(str_idx);
    } else {
      for (int str_idx: *strings_subset)
        for (const auto& pattern: PatternFunc(trimString<trim_direction>(strings[str_idx], trim_size), nullptr)) 
          pat2str[pattern].push_back(str_idx);
    }
  }
}


#endif // MAP_PATTERNS_HPP