#include "vulkan/rendering/utils.hh"
#include <iostream>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "buffer_manager.hh"
#include "utils/stb_image.h"
#include "vulkan/context.hh"
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

static bool hasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void BufferManager::createImage(Image &image, VkFormat format,
                                VkImageTiling tiling, VkImageUsageFlags usage,
                                VkMemoryPropertyFlags properties) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D; // 1 2 or 3D
  // specify dimensions
  imageInfo.extent.width = image.width;
  imageInfo.extent.height = image.height;
  imageInfo.extent.depth = 1;
  // not using mipmap (for now)
  imageInfo.mipLevels = 1;
  // not an array
  imageInfo.arrayLayers = 1;

  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  // only relevant for images that will be used as attachments
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

  // only used by graphic queue
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.flags = 0; // Optional, used for sparsed images

  if (vkCreateImage(context.device, &imageInfo, nullptr, &image.textureImage) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(context.device, image.textureImage,
                               &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(context.device, &allocInfo, nullptr,
                       &image.textureImageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  image.format = format;
  image.layout = VK_IMAGE_LAYOUT_UNDEFINED;
  vkBindImageMemory(context.device, image.textureImage,
                    image.textureImageMemory, 0);
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

  createBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
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

void BufferManager::copyBufferToImage(VkBuffer buffer, Image &image) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();
  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {static_cast<uint32_t>(image.width),
                        static_cast<uint32_t>(image.height), 1};
  vkCmdCopyBufferToImage(commandBuffer, buffer, image.textureImage,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  endSingleTimeCommands(commandBuffer);
}

void BufferManager::transitionImageLayout(Image &image,
                                          VkImageLayout newLayout) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = image.layout;
  barrier.newLayout = newLayout;

  // Used to transfer queue family ownership
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = image.textureImage;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;   // No mipmap for now
  barrier.subresourceRange.levelCount = 1;     // Level of mipmap
  barrier.subresourceRange.baseArrayLayer = 0; // Not an array
  barrier.subresourceRange.layerCount = 1;     // Number of array element

  barrier.srcAccessMask = 0; // TODO
  barrier.dstAccessMask = 0; // TODO

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (hasStencilComponent(image.format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  if (image.layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    // In case of transition from undefined to optimal for memcpy
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    // earliest pseudo-stage for barrier
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    // pseudo-stage for transfer
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (image.layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

      if (hasStencilComponent(image.format)) {
        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
      }
    } else {
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

  } else if (image.layout == VK_IMAGE_LAYOUT_UNDEFINED &&
             newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);
  image.layout = newLayout;
  endSingleTimeCommands(commandBuffer);
}

void BufferManager::createImageView(Image &image,
                                    VkImageAspectFlags aspectFlags) {
  VkImageViewCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  createInfo.image = image.textureImage;
  createInfo.format = image.format;
  createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  // Level map related
  createInfo.subresourceRange.baseMipLevel = 0;
  createInfo.subresourceRange.levelCount = 1;
  createInfo.subresourceRange.aspectMask = aspectFlags; // eg color or depth bit
  createInfo.subresourceRange.layerCount = 1;
  createInfo.subresourceRange.baseArrayLayer = 0;
  // Optional (VK_COMPONENT_SWIZZLE_IDENTITY is 0)
  // The components is usefull for remapping the colors when passed to shader
  // One can also set constants (such as 1 or 0)
  createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  if (vkCreateImageView(context.device, &createInfo, nullptr, &image.view) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }
}

void BufferManager::createTextureImageView() {

  for (auto &image : context.images) {
    createImageView(image, VK_IMAGE_ASPECT_COLOR_BIT);
  }
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
