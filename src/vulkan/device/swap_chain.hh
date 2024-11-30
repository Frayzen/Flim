#pragma once
#include <fwd.h>

VkSurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats);

VkPresentModeKHR chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes);

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
