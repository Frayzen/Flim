#include <vulkan/vulkan_core.h>
#include <cstring>

#include "vulkan/context.hh"
#include "vulkan/buffers/buffer_manager.hh"
#include "vulkan/rendering/pipeline_manager.hh"

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
