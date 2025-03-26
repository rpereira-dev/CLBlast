
// =================================================================================================
// This file is part of the CLBlast project. The project is licensed under Apache Version 2.0. This
// project loosely follows the Google C++ styleguide and uses a tab-size of two spaces and a max-
// width of 100 characters per line.
//
// Author(s):
//   Cedric Nugteren <www.cedricnugteren.nl>
//
// This file provides helper functions for time measurement and such.
//
// =================================================================================================

#ifndef CLBLAST_TIMING_H_
#define CLBLAST_TIMING_H_

#include <cstdio>
#include <utility>
#include <vector>
#include <algorithm>
#include <chrono>

#if CUDA_ENERGY
# include <nvml.h>
#endif

#include "utilities/utilities.hpp"

namespace clblast {
// =================================================================================================

# include <time.h>

static inline uint64_t
get_nanotime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000000000) + (uint64_t) ts.tv_nsec;
}

template <typename F>
int TimeFunction(const size_t num_runs, F const &function,
                    double & time_min_ms, double & total_time_ms, double & total_J
) {

  # if CUDA_ENERGY
  // Get the handle for the first device
  nvmlDevice_t nvdevice;
  nvmlReturn_t result = nvmlDeviceGetHandleByIndex(0, &nvdevice);
  if (NVML_SUCCESS != result) {
      printf("Failed to get handle for device 0: %s\n", nvmlErrorString(result));
      return 1;
  }
  # endif

  // warm-up
  for (int i = 0 ; i < 5 ; ++i)
      function();

  // init accumulators
  time_min_ms   = std::numeric_limits<double>::infinity();
  total_time_ms = 0.0;
  total_J       = 0.0;

  # if CUDA_ENERGY
  unsigned long long int mj0;
  result = nvmlDeviceGetTotalEnergyConsumption(nvdevice, &mj0);
  if (NVML_SUCCESS != result) {
      printf("Failed to get total energy consumption: %s\n", nvmlErrorString(result));
      return 1;
  }
  # endif /* CUDA_ENERGY */

  // run 'n' times,
  const uint64_t t0 = get_nanotime();
  for (size_t i = 0 ; i < num_runs ; ++i)
  {
      const uint64_t t0 = get_nanotime();
      function();
      const uint64_t tf = get_nanotime();
      const uint64_t dt = (double) (tf - t0) / (double)1e9 * (double)1e3;
      if (dt < time_min_ms)
          time_min_ms = dt;
  }
  const uint64_t tf = get_nanotime();
  total_time_ms = (double) (tf - t0) / (double)1e9 * (double)1e3;

  # if CUDA_ENERGY
  unsigned long long int mj1;
  result = nvmlDeviceGetTotalEnergyConsumption(nvdevice, &mj1);
  if (NVML_SUCCESS != result) {
      printf("Failed to get total energy consumption: %s\n", nvmlErrorString(result));
      return 1;
  }

  // Retrieves total energy consumption for this GPU in millijoules (mJ) since the driver was last reloaded
  // For Volta or newer fully supported devices.
  total_J = (mj1 - mj0) / (double)1e3;

  # endif /* CUDA_ENERGY */
  return 0;
}

// =================================================================================================

int RunKernelTimed(const size_t num_runs, Kernel &kernel, Queue &queue, const Device &device,
                      std::vector<size_t> global, const std::vector<size_t> &local,
                      double & time_min_ms, double & total_time_ms, double & total_J);

int TimeKernel(const size_t num_runs, Kernel &kernel, Queue &queue, const Device &device,
                  std::vector<size_t> global, const std::vector<size_t> &local,
                  const bool silent, double & time_min_ms, double & total_time_ms, double & total_J);

// =================================================================================================

using Timing = std::pair<size_t, double>;

template <typename T, typename F>
std::vector<Timing> TimeRoutine(const size_t from, const size_t to, const size_t step,
                                const size_t num_runs, Queue& queue,
                                const std::vector<Buffer<T>>& buffers, F const &routine) {
  auto timings = std::vector<Timing>();
  printf("|  value |         time |\n");
  printf("x--------x--------------x\n");
  for (auto value = from; value < to; value += step) {
    printf("| %6zu |", value);
    try {
      const auto FunctionToTune = [&]() { routine(value, queue, buffers); };
      double time_min_ms;
      double total_time_ms;
      double total_J;
      int err = TimeFunction(num_runs, FunctionToTune, time_min_ms, total_time_ms, total_J);
      printf(" %9.2lf ms |\n", time_min_ms);
      timings.push_back({value, time_min_ms});
    }
    catch (...) {
      const auto status_code = DispatchExceptionCatchAll(true);
      printf("  error %-5d |\n", static_cast<int>(status_code));
      timings.push_back({value, -1.0}); // invalid
    }
  }
  printf("x--------x--------------x\n");
  return timings;
}

// =================================================================================================
} // namespace clblast

// CLBLAST_TIMING_H_
#endif
