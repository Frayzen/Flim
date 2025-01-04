#pragma once

#include "vulkan/context.hh"
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsAndComputeFamily;
  std::optional<uint32_t> presentFamily;
  bool isComplete() {
    return graphicsAndComputeFamily.has_value() && presentFamily.has_value();
  }
};

inline QueueFamilyIndices findQueueFamilies(VulkanContext &context,
                                            VkPhysicalDevice device) {
  // Verify what kind of queues the device can handle
  QueueFamilyIndices indices;
  // Assign index to queue families that could be found
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
        (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
      indices.graphicsAndComputeFamily = i;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, context.surface,
                                         &presentSupport);
    if (presentSupport) {
      indices.presentFamily = i;
    }
    if (indices.isComplete())
      break;
    i++;
  }

  return indices;
}
