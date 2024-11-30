#include "device_manager.hh"
#include "vulkan/context_holder.hh"
#include "vulkan/device/swap_chain.hh"
#include <set>
#include <stdexcept>

// SWAP CHAIN

void DeviceManager::createSwapChain() {
  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode =
      chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
  uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
  if (swapChainSupport.capabilities.maxImageCount > 0 /* 0 = no limit */ &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }
  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = getContext().surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;

  // It is also possible to render images to a separate image first to
  // perform operations like post-processing. In that case one may use a value
  // like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to
  // transfer the rendered image to a swap chain image.
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = findQueueFamilies(getContext().physicalDevice);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                   indices.presentFamily.value()};

  // We need to specify how to handle swap chain images that will be used across
  // multiple queue families. That will be the case in our application if the
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

  if (vkCreateSwapchainKHR(getContext().device, &createInfo, nullptr,
                           &getContext().swapChain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }
}

// LOGICAL DEVICE CREATION

static void setupQueues(QueueFamilyIndices &indices) {
  vkGetDeviceQueue(getContext().device, indices.graphicsFamily.value(), 0,
                   &getContext().queues.graphicsQueue);
  vkGetDeviceQueue(getContext().device, indices.presentFamily.value(), 0,
                   &getContext().queues.presentQueue);
}

void DeviceManager::createLogicalDevice() {
  VkPhysicalDevice physicalDevice = getContext().physicalDevice;
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                            indices.presentFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  // Specify the feature the app is using by setting them to VK_TRUE
  VkPhysicalDeviceFeatures deviceFeatures{};

  // Create the logical device
  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());

  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr,
                     &getContext().device) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  setupQueues(indices);
}
