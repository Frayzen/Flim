#pragma once

#include "vulkan/device/device_manager.hh"
#include "vulkan/graphics/pipeline.hh"
#include <vulkan/vulkan_core.h>

class CommandPool {
public:
  CommandPool(DeviceManager &device_manager, Pipeline &pipeline)
      : device_manager(device_manager), pipeline(pipeline) {};
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();
  void drawFrame();
  void cleanup();

private:
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

  VkCommandPool commandPool = {};
  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;

  DeviceManager &device_manager;
  Pipeline &pipeline;
};
