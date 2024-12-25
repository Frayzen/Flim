#pragma once

#include <cstdint>
#include <fwd.hh>
#include <vulkan/vulkan_core.h>

namespace Flim {
class Renderer;
};
class VulkanContext {
public:
  uint32_t currentImage;
  VkInstance instance;
  GLFWwindow *window;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  VkSurfaceKHR surface;
  Queues queues;
  CommandPool commandPool;
  Pipeline pipeline;
  SwapChain swapChain;
  Buffer stagingBuffer;
  Buffer indexBuffer;
  Buffer vertexBuffer;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  std::vector<Image> images;
  Image depthImage;
  Flim::Renderer *renderer;
} extern context;
