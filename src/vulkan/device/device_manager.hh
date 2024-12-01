#pragma once

#include "vulkan/base_manager.hh"
#include <vulkan/vulkan_core.h>

class DeviceManager : public BaseManager {
public:
  DeviceManager(VulkanContext &context) : BaseManager(context) {};

  void pickPhysicalDevice();
  void createLogicalDevice();

private:
  bool isDeviceSuitable(VkPhysicalDevice device);
};
