#pragma once

#include "api/render/mesh.hh"
#include "vulkan/context.hh"
#include <vulkan/vulkan_core.h>

class CommandPoolManager {
public:
  CommandPoolManager() : commandPool(context.commandPool) {}
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();
  bool acquireFrame(); // return if the swap chain is no longer adequate
  void renderFrame(const Flim::Mesh &mesh);
  bool submitFrame(bool framebufferResized); // return if the swap chain is no
                                             // longer adequate
  void cleanup();

private:
  void recordCommandBuffer(VkCommandBuffer commandBuffer,
                           const Flim::Mesh &mesh);
  CommandPool &commandPool;
};
