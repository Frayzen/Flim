#pragma once

#include "vulkan/computing/computer.hh"
#include "vulkan/context.hh"
#include "vulkan/rendering/renderer.hh"
#include <vulkan/vulkan_core.h>

class CommandPoolManager {
public:
  CommandPoolManager() : commandPool(context.commandPool) {}
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();
  bool acquireFrame(); // return if the swap chain is no longer
                       // adeQuaternionfernionfe
  void recordCommandBuffer(const Renderer &renderer);
  void recordCommandBuffer(const Computer &computer);
  bool submitFrame(bool framebufferResized); // return if the swap chain is no
                                             // longer adeQuaternionfernionfe
  void cleanup();

private:
  void createCommandBuffer(std::vector<VkCommandBuffer> &buffers);
  CommandPool &commandPool;
};
