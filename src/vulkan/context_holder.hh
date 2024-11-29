#pragma once
#include <fwd.h>

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
  Queues queues;
};

VulkanContext &getContext();
