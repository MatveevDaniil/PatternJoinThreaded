#ifndef TIMING_IMPL_HPP
#define TIMING_IMPL_HPP

#include <fstream>
#include <iostream>
#include <chrono>


template <typename Func, typename... Args>
void measure_time(std::ofstream& ofs, const std::string& label, Func&& func, Args&&... args) {
  const int iterations = 1;
  double total_duration = 0.0;

  for (int i = 0; i < iterations; ++i) {
    auto start = std::chrono::high_resolution_clock::now();
    std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_duration += elapsed.count();
  }

  double average_duration = total_duration / iterations;
  ofs << label << "," << average_duration << std::endl;
  ofs.flush();
};

#endif // TIMING_IMPL_HPP
