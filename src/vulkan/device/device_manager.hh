#pragma once

#include <vulkan/vulkan_core.h>

class DeviceManager {
public:
  DeviceManager() = default;

  void pickPhysicalDevice();
  void createLogicalDevice();

private:
  bool isDeviceSuitable(VkPhysicalDevice device);
};
