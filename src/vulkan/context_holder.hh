#pragma once
#include <fwd.h>
#include <vulkan/vulkan_core.h>

typedef struct Queues {
  VkQueue presentQueue;
  VkQueue graphicsQueue;
} Queues;

class VulkanContext {
public:
  VkInstance instance;
  GLFWwindow *window;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  VkSurfaceKHR surface;
  VkSwapchainKHR swapChain;
  Queues queues;
};

VulkanContext &getContext();
