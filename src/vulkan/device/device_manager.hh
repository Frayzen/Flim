#pragma once

#include "vulkan/base_manager.hh"
#include <vulkan/vulkan_core.h>

class DeviceManager : public BaseManager {
public:
  DeviceManager(VulkanContext &context)
      : BaseManager(context), swapChainSupport(context.swapChainSupport) {}

  void pickPhysicalDevice();
  void createLogicalDevice();

private:
  SwapChainSupportDetails &swapChainSupport;
  bool isDeviceSuitable(VkPhysicalDevice device);
  void querySwapChainSupport(VkPhysicalDevice &device);
};
