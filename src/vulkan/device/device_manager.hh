#pragma once

#include <vulkan/vulkan_core.h>

namespace Flim {

class DeviceManager {
public:
  DeviceManager() = default;
  ~DeviceManager();

  void pickPhysicalDevice();
  void createLogicalDevice();

private:
  bool isDeviceSuitable(VkPhysicalDevice device);
};
} // namespace Flim
