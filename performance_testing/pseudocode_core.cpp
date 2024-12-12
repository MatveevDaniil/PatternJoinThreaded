#pragma omp parallel for
for (num : input)
  #pragma omp critical // only for serial sets
  set.insert(num);
