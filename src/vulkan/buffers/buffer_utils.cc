#include "buffer_utils.hh"
#include "vulkan/context.hh"
#include <cstring>
#include <fwd.hh>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#include "utils/backend.hh"

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(context.physicalDevice, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("failed to find suitable memory type!");
}

VkCommandBuffer beginSingleTimeCommands() {
  // Create the command buffer
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = context.commandPool.pool;
  allocInfo.commandBufferCount = 1;
  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(context.device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // Specify that this command is intended to be used once and awaited
  // immediately
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
  // End of the commands !
  vkEndCommandBuffer(commandBuffer);

  // Submit the command buffer (containing only one command)
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  // Submit it in the graphic queue (or a queue that supports transfer operation
  // = has VK_QUEUE_TRANSFER_BIT)
  vkQueueSubmit(context.queues.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

  // Wait for the end of the command execution
  vkQueueWaitIdle(context.queues.graphicsQueue);

  // Clean the command buffer afterward !
  vkFreeCommandBuffers(context.device, context.commandPool.pool, 1,
                       &commandBuffer);
}

// Buffer class related -----

void Buffer::populate(void *data) const {
  Buffer stagingBuffer("Populate staging buffer of " + name, size,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       false);
  stagingBuffer.map();
  memcpy(stagingBuffer.mappedPtr, data, (size_t)size);
  stagingBuffer.unmap();
  copy(stagingBuffer);
  // stagingBuffer.destroy();
}

void Buffer::copy(const Buffer &from) const {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  // Create the CmdCopyBuffer command
  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = size;
  // Append it to the list
  vkCmdCopyBuffer(commandBuffer, from.buffer, buffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer);
}

void Buffer::map() {
  if (mappedPtr != nullptr)
    return;
  vkMapMemory(context.device, bufferMemory, 0, size, 0, &mappedPtr);
}

void Buffer::unmap() {
  if (mappedPtr == nullptr)
    return;
  vkUnmapMemory(context.device, bufferMemory);
  mappedPtr = nullptr;
}

Buffer::~Buffer() {
  if (external) {
#ifdef FLIM_HIP
    // Note: hipFree(externalPtr) would CRASH - externalPtr wasn't allocated
    // with hipMalloc The pointer is managed by the external memory object
    HIP_CHECK(hipDestroyExternalMemory(extMem)); // This invalidates externalPtr
    // File descriptor ownership transferred to HIP, don't close it here
    // close(externalFd);  // REMOVE THIS - HIP owns the FD now
#elif defined(FLIM_CUDA)
    CUDA_CHECK(
        cudaDestroyExternalMemory(extMem)); // This invalidates externalPtr
    // Don't close externalFd - CUDA driver owns it
    // close(externalFd);  // REMOVE THIS
#endif
  }
  if (mappedPtr)
    unmap();
  vkDestroyBuffer(context.device, buffer, nullptr);
  vkFreeMemory(context.device, bufferMemory, nullptr);
}

void Buffer::create(VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties) {
  void *pNextMem = nullptr;
  void *pNextBuf = nullptr;

  if (external) {
    static VkExportMemoryAllocateInfo exportInfo{};
    exportInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
    exportInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    pNextMem = &exportInfo;

    static VkExternalMemoryBufferCreateInfo externalBufferInfo{};
    externalBufferInfo.sType =
        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
    externalBufferInfo.handleTypes =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    pNextBuf = &externalBufferInfo;
  }

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  bufferInfo.pNext = pNextBuf;

  if (vkCreateBuffer(context.device, &bufferInfo, nullptr, &buffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create buffer!");
  }

  VkDebugUtilsObjectNameInfoEXT debugInfo{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .pNext = NULL,
      .objectType = VK_OBJECT_TYPE_BUFFER,
      .objectHandle = (uint64_t)buffer,
      .pObjectName = name.c_str(),
  };

  auto vkSetDebugUtilsObjectNameEXT =
      (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(
          context.instance, "vkSetDebugUtilsObjectNameEXT");
  vkSetDebugUtilsObjectNameEXT(context.device, &debugInfo);

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(context.device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);
  allocInfo.pNext = pNextMem;

  if (vkAllocateMemory(context.device, &allocInfo, nullptr, &bufferMemory) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(context.device, buffer, bufferMemory, 0);

  if (external) {

    // Export the memory handle
    VkMemoryGetFdInfoKHR getFdInfo = {};
    getFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    getFdInfo.memory = bufferMemory;
    getFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    auto vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)vkGetInstanceProcAddr(
        context.instance, "vkGetMemoryFdKHR");

    assert(vkGetMemoryFdKHR(context.device, &getFdInfo, &externalFd) ==
           VK_SUCCESS);
#ifdef FLIM_HIP
    HIP_CHECK(hipSetDevice(0));

    // Import the memory handle into HIP
    hipExternalMemoryHandleDesc extMemHandleDesc = {};
    extMemHandleDesc.type = hipExternalMemoryHandleTypeOpaqueFd;
    extMemHandleDesc.handle.fd = externalFd;
    extMemHandleDesc.size = size;

    HIP_CHECK(hipImportExternalMemory(&extMem, &extMemHandleDesc));

    // Map the external memory to a HIP buffer
    hipExternalMemoryBufferDesc bufferDesc = {};
    bufferDesc.offset = 0;
    bufferDesc.size = size;
    bufferDesc.flags = 0;

    HIP_CHECK(
        hipExternalMemoryGetMappedBuffer(&externalPtr, extMem, &bufferDesc));

#elif defined(FLIM_CUDA)
    // Set CUDA device
    CUDA_CHECK(cudaSetDevice(0));

    // Import the memory handle into CUDA
    cudaExternalMemoryHandleDesc extMemHandleDesc = {};
    extMemHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueFd;
    extMemHandleDesc.handle.fd = externalFd;
    extMemHandleDesc.size = size;

    CUDA_CHECK(cudaImportExternalMemory(&extMem, &extMemHandleDesc));

    // Map the external memory to a CUDA buffer
    cudaExternalMemoryBufferDesc bufferDesc = {};
    bufferDesc.offset = 0;
    bufferDesc.size = size;
    bufferDesc.flags = 0; // Must be 0 for now (reserved for future use)

    CUDA_CHECK(
        cudaExternalMemoryGetMappedBuffer(&externalPtr, extMem, &bufferDesc));
#endif
  }
}
