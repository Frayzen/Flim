#pragma once

#include "vulkan/base_manager.hh"
#include "vulkan/context.hh"
class BufferManager : public BaseManager {
public:
  BufferManager(VulkanContext &context) : BaseManager(context) {};

  void createVertexBuffer();
  void createIndexBuffer();
  void cleanup();

private:
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};
