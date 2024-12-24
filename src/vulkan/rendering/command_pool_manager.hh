#pragma once

#include "vulkan/context.hh"
#include <vulkan/vulkan_core.h>

class CommandPoolManager {
public:
  CommandPoolManager() : commandPool(context.commandPool) {}
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();
  bool acquireFrame(); // return if the swap chain is no longer adequate
  void renderFrame(uint32_t numberIndices);
  bool submitFrame(bool framebufferResized); // return if the swap chain is no
                                             // longer adequate
  void cleanup();

private:
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex,
                           uint32_t numberIndices);
  CommandPool &commandPool;
};
