#include "device_manager.hh"

#include "vulkan/device/device_utils.hh"
#include <set>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

// LOGICAL DEVICE CREATION

static void setupQueues(VulkanContext &context, QueueFamilyIndices &indices) {
  vkGetDeviceQueue(context.device, indices.graphicsAndComputeFamily.value(), 0,
                   &context.queues.graphicsQueue);
  vkGetDeviceQueue(context.device, indices.presentFamily.value(), 0,
                   &context.queues.presentQueue);
  vkGetDeviceQueue(context.device, indices.presentFamily.value(), 0,
      &context.queues.computeQueue);
}

void DeviceManager::createLogicalDevice() {
  VkPhysicalDevice physicalDevice = context.physicalDevice;
  QueueFamilyIndices indices = findQueueFamilies(context, physicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {
      indices.graphicsAndComputeFamily.value(), indices.presentFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  // specify dynamic rendering
  VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{};
  dynamicRenderingFeature.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
  dynamicRenderingFeature.dynamicRendering = VK_TRUE;

  // specify device address feature
  VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeat = {};
  bufferDeviceAddressFeat.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
  bufferDeviceAddressFeat.bufferDeviceAddress = VK_TRUE;
  bufferDeviceAddressFeat.pNext = &dynamicRenderingFeature;

  // Specify the feature the context is using by setting them to VK_TRUE
  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceFeatures.fillModeNonSolid = VK_TRUE;
  deviceFeatures.wideLines = VK_TRUE;

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

  createInfo.pNext = &bufferDeviceAddressFeat;

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &context.device) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  setupQueues(context, indices);
}
