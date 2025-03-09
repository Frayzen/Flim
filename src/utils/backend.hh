#pragma once

#if defined(__HIP_PLATFORM_AMD__) or defined(__HIP_PLATFORM_NVIDIA__)
#define HIP

#include <stdio.h>
#if defined(__HIP_PLATFORM_AMD__)
#include <hip/amd_detail/amd_hip_runtime.h>
#elif defined(__HIP_PLATFORM_NVIDIA__)
#include <hip/nvidia_detail/amd_nvidia_runtime.h>
#endif

#include <iostream>
#include <hip/hip_ext.h>

#define HIP_CHECK(expression)                                                  \
  {                                                                            \
    const hipError_t status = expression;                                      \
    if (status != hipSuccess) {                                                \
      std::cerr << "HIP error " << status << ": " << hipGetErrorString(status) \
                << " at " << __FILE__ << ":" << __LINE__ << std::endl;         \
    }                                                                          \
  }
#endif
