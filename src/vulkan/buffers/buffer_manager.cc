#include "buffer_manager.hh"
#include <cstddef>
#include <stdexcept>

uint32_t BufferManager::findMemoryType(uint32_t typeFilter,
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

void BufferManager::createBuffer(VulkanContext &context, VkDeviceSize size,
                                 VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 Buffer &buffer) {
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
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(context.device, &allocInfo, nullptr,
                       &buffer.bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(context.device, buffer.buffer, buffer.bufferMemory, 0);
}

VkCommandBuffer BufferManager::beginSingleTimeCommands() {
  //
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

void BufferManager::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
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

void BufferManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                               VkDeviceSize size) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  // Create the CmdCopyBuffer command
  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = size;
  // Append it to the list
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer);
}

void BufferManager::cleanup() {

  for (auto &image : context.images) {
    vkDestroySampler(context.device, image.sampler, nullptr);
    vkDestroyImageView(context.device, image.view, nullptr);
    vkDestroyImage(context.device, image.textureImage, nullptr);
    vkFreeMemory(context.device, image.textureImageMemory, nullptr);
  }
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(context.device, uniformBuffers[i].buffer, nullptr);
    vkFreeMemory(context.device, uniformBuffers[i].bufferMemory, nullptr);
  }
  vkDestroyDescriptorPool(context.device, context.descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(context.device, context.descriptorSetLayout,
                               nullptr);

  vkDestroyBuffer(context.device, context.indexBuffer.buffer, nullptr);
  vkFreeMemory(context.device, context.indexBuffer.bufferMemory, nullptr);

  vkDestroyBuffer(context.device, context.vertexBuffer.buffer, nullptr);
  vkFreeMemory(context.device, context.vertexBuffer.bufferMemory, nullptr);
}
