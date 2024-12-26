#include "buffer_manager.hh"
#include "vulkan/buffers/texture_utils.hh"
#include "vulkan/context.hh"
#include "vulkan/rendering/utils.hh"

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
