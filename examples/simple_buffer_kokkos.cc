#include "api/flim_api.hh"
#include "vulkan/buffers/buffer_utils.hh"
#include <Kokkos_Core.hpp>
#include <Kokkos_Core_fwd.hpp>
#include <decl/Kokkos_Declare_OPENMP.hpp>
#include <hip/amd_detail/amd_hip_runtime.h>
#include <iostream>
#include <setup/Kokkos_Setup_HIP.hpp>
#include <vulkan/vulkan_core.h>

#include <hip/hip_ext.h>
#define HIP_CHECK(expression)                                                  \
  {                                                                            \
    const hipError_t status = expression;                                      \
    if (status != hipSuccess) {                                                \
      std::cerr << "HIP error " << status << ": " << hipGetErrorString(status) \
                << " at " << __FILE__ << ":" << __LINE__ << std::endl;         \
    }                                                                          \
  }

int main() {
  Kokkos::initialize();

  Flim::FlimAPI api = Flim::FlimAPI::init();
  {
    auto bufferSize = 10 * sizeof(float);
    Buffer buffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 0, true);


    // Now `hipBuffer` can be used in HIP kernels

    auto tst = buffer.getView<float>();
    Kokkos::View<float *, Kokkos::HIPSpace> other("OKAY", 10);
    std::cout << "GOT KOK ADDR " << other.data() << '\n';
    std::cout << "GOT HIP ADDR" << tst.data() << '\n';

    Kokkos::parallel_for(
        "Check", 10, KOKKOS_LAMBDA(const int i) { tst(i) = i * 3.14; });

    Kokkos::View<float *, Kokkos::DefaultHostExecutionSpace> host("host", 10);
    Kokkos::deep_copy(host, tst);
    for (int i = 0; i < 10; i++) {
      std::cout << "VAL " << i << " is " << host(i) << '\n';
    }


  }
  Kokkos::finalize();
  return 0;
}
