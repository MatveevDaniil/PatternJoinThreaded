#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "sim_search_semi_patterns.hpp"
#include "duplicates_search.hpp"

namespace py = pybind11;

PYBIND11_MODULE(_core, module) {
  module.def("join", [](const std::vector<std::string>& a,
    const std::vector<std::string>& b, int cutoff, char metric,
    int threads) {
    if (cutoff < 0 || cutoff > 2)
      throw std::invalid_argument("cutoff must be 0, 1 or 2");
    if (threads < 0)
      throw std::invalid_argument("threads must be nonnegative");
    get_distance_k(metric);
    py::gil_scoped_release release;
    int_pair_set pairs;
    if (cutoff == 0)
      duplicates_search(a, b, pairs);
    else
      sim_search_semi_patterns(a, b, cutoff, metric, pairs, threads);
    std::vector<std::pair<int, int>> result(pairs.begin(), pairs.end());
    std::sort(result.begin(), result.end());
    return result;
  });
}
