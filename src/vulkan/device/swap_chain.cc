#include "swap_chain.hh"
#include "vulkan/context_holder.hh"
#include <algorithm>
#include <limits>

VkSurfaceFormatKHR chooseSwapSurfaceFormat(
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
VkPresentModeKHR chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR; // Guaranteed to exists
}

// Swap extent is the resolution of the swap chain images

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(getContext().window, &width, &height);

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
