#pragma once

#include "vulkan/rendering/rendering_context.hh"
#include <cstdint>
#include <fwd.hh>
#include <vulkan/vulkan_core.h>

namespace Flim {

class RenderParams;
};
class VulkanContext {
public:
  uint32_t currentImage;
  uint32_t currentUpdate;
  VkInstance instance;
  GLFWwindow *window;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  VkSurfaceKHR surface;
  Queues queues;
  CommandPool commandPool;
  SwapChain swapChain;
  Image depthImage;
  RenderingContext rctx;
} extern context;
