#ifndef HASHMAP_CONTAINERS_HPP
#define HASHMAP_CONTAINERS_HPP

#include "../thirdparty/unordered_dense.h"
#include "../thirdparty/small_vector.hpp"
#include "../thirdparty/gtl/phmap.hpp"

using str2int = ankerl::unordered_dense::map<std::string, int>;
using ints = gch::small_vector<int>;
using str2ints = ankerl::unordered_dense::map<std::string, ints>;
// using str2ints = gtl::parallel_flat_hash_map<std::string, ints>;
// using int_pair_set = gtl::parallel_flat_hash_set_m<std::pair<int, int>>;
using int_pair_set = ankerl::unordered_dense::set<std::pair<int, int>>;
using str_pair_set = ankerl::unordered_dense::set<std::pair<std::string, std::string>>;
using str_pair_set = ankerl::unordered_dense::set<std::pair<std::string, std::string>>;

#endif // HASHMAP_CONTAINERS_HPP