#pragma once

#include "fwd.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>

typedef struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
} SwapChainSupportDetails;

typedef struct Queues {
  VkQueue presentQueue;
  VkQueue graphicsQueue;
} Queues;

typedef struct CommandPool {
  VkCommandPool pool = {};
  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
} CommandPool;

typedef struct Pipeline {
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
  VkPipelineLayout pipelineLayout;
  VkRenderPass renderPass;
  VkPipeline graphicsPipeline;
} Pipeline;

typedef struct SwapChain {
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkSwapchainKHR swapChain;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
} SwapChain;

typedef struct Buffer {
  VkDeviceMemory bufferMemory;
  VkBuffer buffer;
} Buffer;

typedef struct Image {
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  int width, height;
} Image;

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
  Buffer uniformBuffer;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  std::vector<Image> images;
};