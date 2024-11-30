#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
const uint32_t width = 800;
const uint32_t height = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


/* #define DEBUG_LEVEL VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT */
/* #define DEBUG_LEVEL VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT */
#define VK_DEBUG_LEVEL VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
/* #define DEBUG_LEVEL VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT */


const int MAX_FRAMES_IN_FLIGHT = 2;
