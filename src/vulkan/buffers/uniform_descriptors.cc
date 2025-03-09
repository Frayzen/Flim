#include "uniform_descriptors.hh"

#include "vulkan/buffers/buffer_utils.hh"
#include "vulkan/buffers/descriptor_holder.hh"
#include "vulkan/buffers/texture_utils.hh"
#include "vulkan/context.hh"
#include <cassert>
#include <fwd.hh>
#include <iostream>
#include <vulkan/vulkan_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include <utils/stb_image.h>

ImageUniDesc::ImageUniDesc(int binding, std::string path, int shaderStage)
    : UniformDescriptor(binding, shaderStage,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
      path(path), imageSetup(false) {
  redundancy = 0;
}

void ImageUniDesc::setup() {
  if (imageSetup)
    return;
  imageSetup = true;
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels,
                              STBI_rgb_alpha);
  bool loaded = pixels != nullptr;
  if (!loaded) {
    texWidth = 1;
    texHeight = 1;
    texChannels = 3;
    pixels = new stbi_uc[4]{255, 255, 255};
    if (path.length() != 0)
      std::cerr << "failed to load texture image '" + path + "' !" << std::endl;
    else
      std::cerr << "Warning: empty path for texture (binding " << binding << ")"
                << std::endl;
  }
  VkDeviceSize imageSize = texWidth * texHeight * 4;
  Buffer staging = Buffer(pixels, imageSize);
  if (loaded)
    stbi_image_free(pixels);
  else
    delete[] pixels;

  image.width = texWidth;
  image.height = texHeight;
  // VK_IMAGE_USAGE_TRANSFER_DST_BIT to be able to copy
  // VK_IMAGE_USAGE_SAMPLED_BIT to be able to use it in shader
  createImage(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  transitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  copyBufferToImage(staging.getVkBuffer(), image);
  transitionImageLayout(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  staging.destroy();
  // Image view
  createImageView(image, VK_IMAGE_ASPECT_COLOR_BIT);
  // Sampler
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
  if (vkCreateSampler(context.device, &samplerInfo, nullptr, &image.sampler) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create texture sampler!");
  }
}

VkWriteDescriptorSet ImageUniDesc::getDescriptor(DescriptorHolder &holder,
                                                 int i) {
  assert(imageSetup);
  static VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = image.view;
  imageInfo.sampler = image.sampler;
  VkWriteDescriptorSet descriptor{};
  descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor.dstSet = holder.descriptorSets[i];
  descriptor.dstBinding = binding;
  descriptor.dstArrayElement = 0;
  descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptor.descriptorCount = 1;
  descriptor.pImageInfo = &imageInfo;
  return descriptor;
}

void ImageUniDesc::cleanup() {
  if (!imageSetup)
    return;
  vkDestroySampler(context.device, image.sampler, nullptr);
  vkDestroyImageView(context.device, image.view, nullptr);
  vkDestroyImage(context.device, image.textureImage, nullptr);
  vkFreeMemory(context.device, image.textureImageMemory, nullptr);
  imageSetup = false;
}

void GeneralUniDesc::setup() {
  assert(bufferSize != 0);
  setupBuffers(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false);
  for (auto &b : getBuffers())
    b->map();
}

VkWriteDescriptorSet GeneralUniDesc::getDescriptor(DescriptorHolder &holder,
                                                   int i) {
  assert(bufferSize != 0);
  bufferInfo.buffer = getBuffer(i)->getVkBuffer();
  bufferInfo.offset = 0;
  bufferInfo.range = bufferSize;
  VkWriteDescriptorSet descriptor{};
  descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor.dstSet = holder.descriptorSets[i];
  descriptor.dstBinding = binding;
  descriptor.dstArrayElement = 0;
  descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptor.descriptorCount = 1;
  descriptor.pBufferInfo = &bufferInfo;
  return descriptor;
}

void GeneralUniDesc::cleanup() { cleanupBuffers(); }

void GeneralUniDesc::update() {
  updateFunction(getBuffer()->getPtr());
};
