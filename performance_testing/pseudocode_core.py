#pragma omp parallel for num_threads(P)
for (size_t i = 0; i < input.size(); ++i)
  set.insert(input[i]);

#pragma omp parallel for num_threads(P)
for (const auto& element : input)
  #pragma omp critical
  set.push_back(element);

#pragma omp parallel for num_threads(P)
for (const auto& element : input)
  #pragma omp critical
  set.insert(element);