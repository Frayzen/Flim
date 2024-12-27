#pragma once

#include "vulkan/context.hh"
#include "vulkan/rendering/renderer.hh"
#include <vulkan/vulkan_core.h>

class CommandPoolManager {
public:
  CommandPoolManager() : commandPool(context.commandPool) {}
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();
  bool acquireFrame(); // return if the swap chain is no longer adequate
  void recordCommandBuffer(const Renderer &renderer);
  bool submitFrame(bool framebufferResized); // return if the swap chain is no
                                             // longer adequate
  void cleanup();

private:
  CommandPool &commandPool;
};
