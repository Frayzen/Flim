#pragma once

#include <fwd.h>

#include "vulkan/base_manager.hh"
#include "vulkan/context.hh"

class SwapChainManager : public BaseManager {

public:
  SwapChainManager(VulkanContext &context)
      : BaseManager(context), swapChain(context.swapChain) {};

  void createSwapChain();
  void recreateSwapChain();
  void cleanup();

private:
  SwapChain &swapChain;

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);

  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
};
