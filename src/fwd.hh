#pragma once

#include <Eigen/StdVector>
#include <Eigen/Core>
#include <Eigen/Eigen>
#include <Eigen/Dense>

using namespace Eigen;

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <consts.hh>

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

typedef struct Image {
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView view;
  VkImageLayout layout;
  VkFormat format;
  VkSampler sampler;
  int width, height;
  uint32_t mipLevels;
} Image;

typedef struct SwapChain {
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;
  /* std::vector<VkFramebuffer> swapChainFramebuffers; */
  VkSwapchainKHR swapChain;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
} SwapChain;

typedef struct Buffer {
  VkDeviceMemory bufferMemory;
  VkBuffer buffer;
} Buffer;
