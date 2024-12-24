#include "buffer_manager.hh"
#include "vulkan/buffers/buffer_utils.hh"
#include "vulkan/buffers/texture_utils.hh"
#include "vulkan/rendering/utils.hh"
#include <iostream>

#include "vulkan/context.hh"
#include <cstring>
#include <vulkan/vulkan_core.h>

void BufferManager::createTextureImageView() {

  for (auto &image : context.images) {
    createImageView(image, VK_IMAGE_ASPECT_COLOR_BIT);
  }
}


void BufferManager::createTextureImage(const std::string &imgPath) {
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(imgPath.c_str(), &texWidth, &texHeight,
                              &texChannels, STBI_rgb_alpha);
  bool loaded = pixels != nullptr;
  if (!loaded) {
    texWidth = 1;
    texHeight = 1;
    texChannels = 3;
    pixels = new stbi_uc[4]{255, 255, 255};
    if (imgPath.length() != 0)
      std::cerr << "failed to load texture image '" + imgPath + "' !"
                << std::endl;
  }
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  Buffer staging;

  createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               staging);

  void *data;
  vkMapMemory(context.device, staging.bufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(context.device, staging.bufferMemory);
  if (loaded)
    stbi_image_free(pixels);
  else
    delete[] pixels;

  context.images.resize(1);

  Image &image = context.images[0];
  image.width = texWidth;
  image.height = texHeight;
  // VK_IMAGE_USAGE_TRANSFER_DST_BIT to be able to copy
  // VK_IMAGE_USAGE_SAMPLED_BIT to be able to use it in shader
  createImage(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  transitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  copyBufferToImage(staging.buffer, image);
  transitionImageLayout(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(context.device, staging.buffer, nullptr);
  vkFreeMemory(context.device, staging.bufferMemory, nullptr);
}


void BufferManager::createTextureSampler() {

  for (auto &image : context.images) {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR; // when oversampling
    samplerInfo.minFilter = VK_FILTER_LINEAR; // when undersampling
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // Apply anisotropy (for rendering object viewed at a sharp angle)
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(context.physicalDevice, &properties);
    samplerInfo.anisotropyEnable = VK_TRUE;
    // for performance
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy / 2;

    // Wheather to use [0, texWidth] or [0.0, 1.0] coords (same for height)
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    // Used for percentage-closer filtering, disabled now
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    // Used for mipmapping
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    if (vkCreateSampler(context.device, &samplerInfo, nullptr,
                        &image.sampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture sampler!");
    }
  }
}

void BufferManager::createDepthResources() {
  VkFormat depthFormat = findDepthFormat(context);
  VkExtent2D &extent = context.swapChain.swapChainExtent;
  context.depthImage.width = extent.width;
  context.depthImage.height = extent.height;
  createImage(context.depthImage, depthFormat, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  createImageView(context.depthImage, VK_IMAGE_ASPECT_DEPTH_BIT);

  transitionImageLayout(context.depthImage,
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}
