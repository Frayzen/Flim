#pragma once

#include "vulkan/context.hh"
#include <cstring>
#include <fwd.hh>

static bool hasStencilComponent(VkFormat format);

void createImage(Image &image, VkFormat format, VkImageTiling tiling,
                 VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

void copyBufferToImage(VkBuffer buffer, Image &image);

void transitionImageLayout(Image &image, VkImageLayout newLayout);

void createImageView(Image &image, VkImageAspectFlags aspectFlags);

