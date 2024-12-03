#ifndef MAP_PATTERNS_HPP
#define MAP_PATTERNS_HPP

#include <omp.h>
#include <vector>
#include <string>
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

  #pragma omp parallel 
  {
  if (strings_subset == nullptr) {
    #pragma omp for
    for (std::string str: strings) {
      for (const auto& pattern: PatternFunc(str, nullptr)) {
        #pragma omp critical
        pat2str[pattern];.push_back(str2idx[str]);
      }
    }
  }
  else {
    if (trim_direction == TrimDirection::No) {
      #pragma omp for
      for (int str_idx: *strings_subset) {
        for (const auto& pattern: PatternFunc(strings[str_idx], nullptr)) {
          #pragma omp critical
          pat2str[pattern];.push_back(str2idx[str]);
        }
      }
    }
    else if (trim_direction == TrimDirection::Mid) {
      MidTrimFunc midTrim = getMidTrimFunc(metric_type);
      #pragma omp for
      for (int str_idx: *strings_subset) {
        for (const auto& pattern: PatternFunc(midTrim(strings[str_idx], trim_part), nullptr)) {
          #pragma omp critical
          pat2str[pattern];.push_back(str2idx[str]);
        }
      }
    } else {
      #pragma omp for
      for (int str_idx: *strings_subset) {
        for (const auto& pattern: PatternFunc(trimString<trim_direction>(strings[str_idx], trim_size), nullptr)) {
          #pragma omp critical
          pat2str[pattern];.push_back(str2idx[str]);
        } 
      }
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