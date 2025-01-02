#pragma once

#include <fwd.hh>
#include "vulkan/context.hh"
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

#define TO_RAD(X) ((X) * 0.5f * M_PI / 180.0f)

inline Quaternionf toQuaternion(Vector3f data)
{
    Quaternionf ans;
    double t0 = cos(data.y() * 0.5);
    double t1 = sin(data.y() * 0.5);
    double t2 = cos(data.z() * 0.5);
    double t3 = sin(data.z() * 0.5);
    double t4 = cos(data.x() * 0.5);
    double t5 = sin(data.x() * 0.5);

    ans.x() = t2 * t4 * t0 + t3 * t5 * t1;
    ans.y() = t3 * t4 * t0 - t2 * t5 * t1;
    ans.z() = t2 * t5 * t0 + t3 * t4 * t1;
    ans.w() = t2 * t4 * t1 - t3 * t5 * t0;
    return ans;
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
