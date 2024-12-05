#pragma once

#include "vulkan/base_manager.hh"
#include "vulkan/context.hh"
#include <vulkan/vulkan_core.h>

class CommandPoolManager : public BaseManager {
public:
  CommandPoolManager(VulkanContext &context)
      : BaseManager(context), commandPool(context.commandPool) {}
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();
  bool acquireFrame(); // return if the swap chain is no longer adequate
  bool renderFrame(bool framebufferResized); // return if the swap chain is no longer adequate
  void cleanup();

private:
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  CommandPool &commandPool;
};
