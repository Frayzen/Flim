#include "vulkan/context.hh"
#include "vulkan/rendering/pipeline_manager.hh"
#include <cstring>
#include <stdexcept>

static uint32_t findMemoryType(VulkanContext &context, uint32_t typeFilter,
                               VkMemoryPropertyFlags properties) {

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

void PipelineManager::createVertexBuffer() {
  // Create buffer
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = sizeof(vertices[0]) * vertices.size();
  bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  Buffer &vertexBuffer = context.vertexBuffer;

  if (vkCreateBuffer(context.device, &bufferInfo, nullptr,
                     &vertexBuffer.buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  // Query memory requirements
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(context.device, vertexBuffer.buffer,
                                &memRequirements);

  // Allocate memory
  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(context, memRequirements.memoryTypeBits,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT allow us to make sure that host memory
  // bound to the device memory always reflect the actual state of the buffer
  if (vkAllocateMemory(context.device, &allocInfo, nullptr,
                       &context.vertexBuffer.bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }
  // Bind the buffer to its memory
  vkBindBufferMemory(context.device, vertexBuffer.buffer,
                     vertexBuffer.bufferMemory, 0);
  void *data;
  vkMapMemory(context.device, vertexBuffer.bufferMemory, 0, VK_WHOLE_SIZE, 0,
              &data); // VK_WHOLE_SIZE is the same as bufferInfo.size
  memcpy(data, vertices.data(), (size_t)bufferInfo.size);
  vkUnmapMemory(context.device, vertexBuffer.bufferMemory);
}
