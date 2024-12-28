#include "buffer_utils.hh"
#include "vulkan/context.hh"
#include <cstring>
#include <fwd.hh>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

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

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
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
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(context.device, &allocInfo, nullptr,
                       &buffer.bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(context.device, buffer.buffer, buffer.bufferMemory, 0);
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

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
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

void populateBufferFromData(Buffer &buffer, VkBufferUsageFlags usage,
                            void *data, size_t dataSize,
                            VkMemoryPropertyFlags properties) {
  Buffer stagingBuffer;
  createBuffer(
      dataSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // required to be passed as source
                                        // in a memory transfer operation
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // means that the state of the
                                                // bound buffer reflect the
                                                // state of the actual buffer
      stagingBuffer);
  void *mapped;
  vkMapMemory(context.device, stagingBuffer.bufferMemory, 0, dataSize, 0,
              &mapped);
  memcpy(mapped, data, (size_t)dataSize);
  vkUnmapMemory(context.device, stagingBuffer.bufferMemory);

  // VK_BUFFER_USAGE_TRANSFER_DST_BIT required to be passed as destination in a
  // memory transfer operation

  createBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, properties,
               buffer);
  copyBuffer(stagingBuffer.buffer, buffer.buffer, dataSize);
  vkDestroyBuffer(context.device, stagingBuffer.buffer, nullptr);
  vkFreeMemory(context.device, stagingBuffer.bufferMemory, nullptr);
}
void destroyBuffer(Buffer &buffer) {
  vkDestroyBuffer(context.device, buffer.buffer, nullptr);
  vkFreeMemory(context.device, buffer.bufferMemory, nullptr);
}
