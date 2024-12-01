#include "vulkan/rendering/buffer_manager.hh"
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

static void createBuffer(VulkanContext &context, VkDeviceSize size,
                         VkBufferUsageFlags usage,
                         VkMemoryPropertyFlags properties, Buffer &buffer) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(context.device, &bufferInfo, nullptr, &buffer.buffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(context.device, buffer.buffer,
                                &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(context, memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(context.device, &allocInfo, nullptr,
                       &buffer.bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(context.device, buffer.buffer, buffer.bufferMemory, 0);
}

void BufferManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                               VkDeviceSize size) {
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

  // Create the CmdCopyBuffer command
  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = size;
  // Append it to the list
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
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

void BufferManager::createVertexBuffer() {
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
  createBuffer(
      context, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // required to be passed as source
                                        // in a memory transfer operation
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // means that the state of the
                                                // bound buffer reflect the
                                                // state of the actual buffer
      context.stagingBuffer);
  void *data;
  vkMapMemory(context.device, context.stagingBuffer.bufferMemory, 0, bufferSize,
              0, &data);
  memcpy(data, vertices.data(), (size_t)bufferSize);
  vkUnmapMemory(context.device, context.stagingBuffer.bufferMemory);

  createBuffer(
      context, bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | // required to be passed as destination
                                         // in a memory transfer operation
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, context.vertexBuffer);
  copyBuffer(context.stagingBuffer.buffer, context.vertexBuffer.buffer,
             bufferSize);
  vkDestroyBuffer(context.device, context.stagingBuffer.buffer, nullptr);
  vkFreeMemory(context.device, context.stagingBuffer.bufferMemory, nullptr);
}

void BufferManager::createIndexBuffer() {
  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  createBuffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               context.stagingBuffer);

  void *data;
  vkMapMemory(context.device, context.stagingBuffer.bufferMemory, 0, bufferSize,
              0, &data);
  memcpy(data, indices.data(), (size_t)bufferSize);
  vkUnmapMemory(context.device, context.stagingBuffer.bufferMemory);

  createBuffer(context, bufferSize,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, context.indexBuffer);

  copyBuffer(context.stagingBuffer.buffer, context.indexBuffer.buffer,
             bufferSize);

  vkDestroyBuffer(context.device, context.stagingBuffer.buffer, nullptr);
  vkFreeMemory(context.device, context.stagingBuffer.bufferMemory, nullptr);
}

void BufferManager::cleanup() {
  vkDestroyBuffer(context.device, context.indexBuffer.buffer, nullptr);
  vkFreeMemory(context.device, context.indexBuffer.bufferMemory, nullptr);

  vkDestroyBuffer(context.device, context.vertexBuffer.buffer, nullptr);
  vkFreeMemory(context.device, context.vertexBuffer.bufferMemory, nullptr);
}
