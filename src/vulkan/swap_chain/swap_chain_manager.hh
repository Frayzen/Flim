#pragma once

#include <fwd.hh>

#include "vulkan/context.hh"

class SwapChainManager {

public:
  SwapChainManager() : swapChain(context.swapChain) {};

  void createSwapChain();
  void cleanup();

private:
  SwapChain &swapChain;

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);

  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
};
