#pragma once

#if defined(__HIP_PLATFORM_AMD__) or defined(__HIP_PLATFORM_NVIDIA__)
#define FLIM_HIP

#include <stdio.h>
#if defined(__HIP_PLATFORM_AMD__)
#include <hip/amd_detail/amd_hip_runtime.h>
#elif defined(__HIP_PLATFORM_NVIDIA__)
#include <hip/nvidia_detail/amd_nvidia_runtime.h>
#endif

#include <hip/hip_ext.h>
#include <iostream>

#define HIP_CHECK(expression)                                                  \
  {                                                                            \
    const hipError_t status = expression;                                      \
    if (status != hipSuccess) {                                                \
      std::cerr << "HIP error " << status << ": " << hipGetErrorString(status) \
                << " at " << __FILE__ << ":" << __LINE__ << std::endl;         \
    }                                                                          \
  }
#elif defined(__CUDACC__) // NVCC compiler macro[citation:6]
#define FLIM_CUDA

#include <cuda_runtime.h>
#include <iostream>

#define CUDA_CHECK(expression)                                                 \
  {                                                                            \
    const cudaError_t status = expression;                                     \
    if (status != cudaSuccess) {                                               \
      std::cerr << "CUDA error " << status << ": "                             \
                << cudaGetErrorString(status) << " at " << __FILE__ << ":"     \
                << __LINE__ << std::endl;                                      \
    }                                                                          \
  }

#endif
