#include "surface_manager.hh"
#include "vulkan/context_holder.hh"
#include <GLFW/glfw3.h>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

void SurfaceManager::createSurface() {
  VkResult err =
      glfwCreateWindowSurface(getContext().instance, getContext().window,
                              nullptr, &getContext().surface);
  if (err != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
}

void SurfaceManager::setupSwapChainImages() {
  uint32_t imageCount;
  vkGetSwapchainImagesKHR(getContext().device, getContext().swapChain,
                          &imageCount, nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(getContext().device, getContext().swapChain,
                          &imageCount, swapChainImages.data());
}

void SurfaceManager::createImageViews() {
  // An image view is quite literally a view into an image. It describes how to
  // access the image and which part of the image to access, for example if it
  // should be treated as a 2D texture depth texture without any mipmapping
  // levels
  assert(swapChainImages.size() != 0);
  swapChainImageViews.resize(swapChainImages.size());
  for (size_t i = 0; i < swapChainImages.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapChainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = getContext().swapChainImageFormat;
    // components field allows to swizzle the color channels around
    // for example, one can map all of the channels to the red channel for a
    // monochrome texture. One can also map constant values of 0 and 1 to a
    // channel. In our case we'll stick to the default mapping
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    // the subresourceRange field describes what the image's purpose is and
    // which part of the image should be accessed. Our images will be used as
    // color targets without any mipmapping levels or multiple layers.
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(getContext().device, &createInfo, nullptr,
                          &swapChainImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create image views!");
    }
  }
}

void SurfaceManager::cleanup() {

  for (auto imageView : swapChainImageViews) {
    vkDestroyImageView(getContext().device, imageView, nullptr);
  }
}
