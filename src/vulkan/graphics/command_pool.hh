#pragma once

#include "vulkan/device/device_manager.hh"
#include "vulkan/graphics/pipeline.hh"
#include <vulkan/vulkan_core.h>

class CommandPool {
public:
  CommandPool(DeviceManager &device_manager, Pipeline &pipeline)
      : device_manager(device_manager), pipeline(pipeline) {};
  void createCommandPool();
  void createCommandBuffer();
  void createSyncObjects();
  void drawFrame();
  void cleanup();

private:
  void recordCommandBuffer(uint32_t imageIndex);

  VkCommandPool commandPool = {};
  VkCommandBuffer commandBuffer = {};

  VkSemaphore imageAvailableSemaphore;
  VkSemaphore renderFinishedSemaphore;
  VkFence inFlightFence;

  DeviceManager &device_manager;
  Pipeline &pipeline;
};
