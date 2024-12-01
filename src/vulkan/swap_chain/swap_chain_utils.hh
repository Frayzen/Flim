#pragma once

#include "vulkan/context.hh"

inline SwapChainSupportDetails querySwapChainSupport(VulkanContext &context,
                                                     VkPhysicalDevice &device) {
  SwapChainSupportDetails swapChainSupport;
  VkSurfaceKHR &surface = context.surface;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &swapChainSupport.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  if (formatCount != 0) {
    swapChainSupport.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         swapChainSupport.formats.data());
  }
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                            nullptr);

  if (presentModeCount != 0) {
    swapChainSupport.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentModeCount,
        swapChainSupport.presentModes.data());
  }
  return swapChainSupport;
}
