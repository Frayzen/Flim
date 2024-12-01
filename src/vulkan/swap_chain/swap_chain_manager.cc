#include "swap_chain_manager.hh"
#include "vulkan/device/device_utils.hh"
#include <algorithm>
#include <iostream>
#include <limits>

VkSurfaceFormatKHR SwapChainManager::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    // format specifies color channels and types
    // colorSpace indicates if the SRGB color space is supported or not
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }
  return availableFormats[0];
}

/*
VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are
   transferred to the screen right away, which may result in tearing.

VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes
   an image from the front of the queue when the display is refreshed and the
   program inserts rendered images at the back of the queue. If the queue is
   full then the program has to wait. This is most similar to vertical sync as
   found in modern games. The moment that the display is refreshed is known as
   "vertical blank".

VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs
   from the previous one if the application is late and the queue was empty at
   the last vertical blank. Instead of waiting for the next vertical blank, the
   image is transferred right away when it finally arrives. This may result in
   visible tearing.

VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of
   the second mode. Instead of blocking the application when the queue is full,
   the images that are already queued are simply replaced with the newer ones.
   This mode can be used to render frames as fast as possible while still
   avoiding tearing, resulting in fewer latency issues than standard vertical
   sync. This is commonly known as "triple buffering", although the existence of
   three buffers alone does not necessarily mean that the framerate is unlocked.
 */
VkPresentModeKHR SwapChainManager::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
#ifndef NDEBUG
      std::cout << "FIFO relaxed present mode chosen" << std::endl;
#endif
      return availablePresentMode;
    }
  }
#ifndef NDEBUG
  std::cout << "FIFO present mode chosen" << std::endl;
#endif
  return VK_PRESENT_MODE_FIFO_KHR; // Guaranteed to exists
}

// Swap extent is the resolution of the swap chain images

VkExtent2D SwapChainManager::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(context.window, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

void SwapChainManager::createSwapChain() {
  auto &swapChainSupport = context.swapChainSupport;
  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
  VkPresentModeKHR presentMode =
      chooseSwapPresentMode(swapChainSupport.presentModes);
  uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
  if (swapChainSupport.capabilities.maxImageCount > 0 /* 0 = no limit */ &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }
  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = context.surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;

  swapChain.swapChainImageFormat = surfaceFormat.format;
  swapChain.swapChainExtent = extent;

  // It is also possible to render images to a separate image first to
  // perform operations like post-processing. In that case one may use a value
  // like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to
  // transfer the rendered image to a swap chain image.
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices =
      findQueueFamilies(context, context.physicalDevice);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                   indices.presentFamily.value()};

  // We need to specify how to handle swap chain images that will be used across
  // multiple queue families. That will be the case in our context if the
  // graphics queue family is different from the presentation queue. We'll be
  // drawing on the images in the swap chain from the graphics queue and then
  // submitting them on the presentation queue.
  if (indices.graphicsFamily !=
      indices.presentFamily) { // If we have 2 different queues
    // An image is owned by one queue family at a time and ownership must be
    // explicitly transferred before using it in another queue family. This
    // option offers the best performance
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    // Images can be used across multiple queue families without explicit
    // ownership transfers
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;     // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }
  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // ignore alpha channel (do not blend
                                         // with other window)

  createInfo.presentMode = presentMode;

  // we don't care about the color of pixels that are obscured, for example
  // because another window is in front of them
  createInfo.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(context.device, &createInfo, nullptr,
                           &context.swapChain.swapChain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }
}

void SwapChainManager::cleanup() {
  for (size_t i = 0; i < swapChain.swapChainFramebuffers.size(); i++) {
    vkDestroyFramebuffer(context.device, swapChain.swapChainFramebuffers[i],
                         nullptr);
  }

  for (size_t i = 0; i < swapChain.swapChainImageViews.size(); i++) {
    vkDestroyImageView(context.device, swapChain.swapChainImageViews[i],
                       nullptr);
  }

  vkDestroySwapchainKHR(context.device, swapChain.swapChain, nullptr);
}
