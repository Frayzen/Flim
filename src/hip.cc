#include "api/flim_api.hh"
#include "fwd.hh"
#include "vulkan/buffers/buffer_utils.hh"
#include "vulkan/context.hh"
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
    Buffer buffer;
    auto bufferSize = 10 * sizeof(float);

    VkExportMemoryAllocateInfo exportInfo{};
    exportInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
    exportInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkExternalMemoryBufferCreateInfo externalBufferInfo{};
    externalBufferInfo.sType =
        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
    externalBufferInfo.handleTypes =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkMemoryAllocateFlagsInfo flags_info{};
    flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    flags_info.pNext = &exportInfo;

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                 VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, buffer, &flags_info,
                 &externalBufferInfo);

    // Export the memory handle
    VkMemoryGetFdInfoKHR getFdInfo = {};
    getFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    getFdInfo.memory = buffer.bufferMemory;
    getFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    auto vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)vkGetInstanceProcAddr(
        context.instance, "vkGetMemoryFdKHR");

    int fd;
    assert(vkGetMemoryFdKHR(context.device, &getFdInfo, &fd) == VK_SUCCESS);

    HIP_CHECK(hipSetDevice(0));

    // Import the memory handle into HIP
    hipExternalMemoryHandleDesc extMemHandleDesc = {};
    extMemHandleDesc.type = hipExternalMemoryHandleTypeOpaqueFd;
    extMemHandleDesc.handle.fd = fd;
    extMemHandleDesc.size = bufferSize;

    hipExternalMemory_t extMem = nullptr;

    HIP_CHECK(hipImportExternalMemory(&extMem, &extMemHandleDesc));

    // Step 3: Map the external memory to a HIP buffer
        hipExternalMemoryBufferDesc bufferDesc = {};
    bufferDesc.offset = 0;
    bufferDesc.size = bufferSize;
    bufferDesc.flags = 0;

    void *hipBuffer;
    HIP_CHECK(hipExternalMemoryGetMappedBuffer(&hipBuffer, extMem, &bufferDesc));

    // Now `hipBuffer` can be used in HIP kernels

    Kokkos::View<float *, Kokkos::HIPSpace> tst((float *)hipBuffer, 10);
    Kokkos::View<float *, Kokkos::HIPSpace> other("OKAY", 10);
    std::cout << "GOT KOK ADDR " << other.data() << '\n';
    std::cout << "GOT HIP ADDR" << hipBuffer << '\n';

    Kokkos::parallel_for(
        "Check", 10, KOKKOS_LAMBDA(const int i) { tst(i) = i * 3.14; });

    Kokkos::View<float *, Kokkos::DefaultHostExecutionSpace> host("host", 10);
    Kokkos::deep_copy(host, tst);
    for (int i = 0; i < 10; i++) {
      std::cout << "VAL " << i << " is " << host(i) << '\n';
    }


    // Clean up
    HIP_CHECK(hipFree(hipBuffer));
    HIP_CHECK(hipDestroyExternalMemory(extMem));
    close(fd);
    destroyBuffer(buffer);
  }
  Kokkos::finalize();
  return 0;
}
