#include "sim_search_patterns.hpp"

void sim_search_patterns(
  const std::vector<std::string>& strings,
  int cutoff,
  char metric,
  str2int& str2idx,
  int_pair_set& out,
  ints* strings_subset,
  bool include_eye
) {
  str_int_set pat_str;
  map_patterns<TrimDirection::No>(strings, cutoff, metric, str2idx, strings_subset, pat_str);
  str2ints pat2str;

  for (auto entry : pat_str) {
    int str_idx2 = entry.second;
    ints& indices = pat2str[entry.first];
    for (auto str_idx1 = indices.begin(); str_idx1 != indices.end(); ++str_idx1) {
      if (*str_idx1 > str_idx2) {
        out.insert({str_idx2, *str_idx1});
      } else {  
        out.insert({*str_idx1, str_idx2});
      }
    }
    indices.push_back(str_idx2);
  }

  if (include_eye)
    for (size_t i = 0; i < strings.size(); i++)
      out.insert({i, i});
}


int sim_search_patterns(
  std::string file_name,
  int cutoff,
  char metric,
  bool include_duplicates
) {
  std::vector<std::string> strings;
  str2int str2idx;
  str2ints str2idxs;
  readFile(file_name, strings, str2idx, include_duplicates, str2idxs);

  int_pair_set out;

  sim_search_patterns(strings, cutoff, metric, str2idx, out, nullptr, true);
  std::string out_file_name = file_name + "_p_" + std::to_string(cutoff) + "_" + metric;
  writeFile(out_file_name, out, strings, str2idxs, include_duplicates);
  return 0;
}