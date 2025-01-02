#pragma once

#include "vulkan/context.hh"
#include <fwd.hh>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

#define TO_RAD(X) ((X) * 0.5f * M_PI / 180.0f)

// angles in radian
inline Quaternionf toQuaternion(float roll, float pitch, float yaw) {
  return AngleAxisf(pitch, Vector3f::UnitX()) *
         AngleAxisf(yaw, Vector3f::UnitY()) *
         AngleAxisf(roll, Vector3f::UnitZ());
}

static VkFormat findSupportedFormat(VulkanContext &context,
                                    const std::vector<VkFormat> &candidates,
                                    VkImageTiling tiling,
                                    VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(context.physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format!");
}

inline VkFormat findDepthFormat(VulkanContext &context) {
  return findSupportedFormat(
      context,
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
