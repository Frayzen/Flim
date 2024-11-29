#pragma once

#include <vulkan/vulkan_core.h>

class DeviceManager {
public:
  void pickPhysicalDevice();
  void createLogicalDevice();
};
