#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

class DeviceManager {
public:
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSwapChain();

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

private:
  struct SwapChainSupportDetails swapChainSupport;
  bool isDeviceSuitable(VkPhysicalDevice device);
  void querySwapChainSupport(VkPhysicalDevice &device);
};
