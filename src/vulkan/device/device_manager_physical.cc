#include "device_manager.hh"
#include "vulkan/context_holder.hh"
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
static bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

void DeviceManager::querySwapChainSupport(VkPhysicalDevice &device) {
  VkSurfaceKHR &surface = getContext().surface;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &swapChainSupport.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  if (formatCount != 0) {
    swapChainSupport.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         swapChainSupport.formats.data());
  }
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                            nullptr);

  if (presentModeCount != 0) {
    swapChainSupport.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentModeCount,
        swapChainSupport.presentModes.data());
  }
}

static int rateDeviceSuitability(VkPhysicalDevice device) {
  int score = 0;
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  // Discrete GPUs have a significant performance advantage
  if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 1000;
  }
  // Maximum possible size of textures affects graphics quality
  score += deviceProperties.limits.maxImageDimension2D;
  return score;
}

QueueFamilyIndices DeviceManager::findQueueFamilies(VkPhysicalDevice device) {
  // Verify what kind of queues the device can handle
  QueueFamilyIndices indices;
  // Assign index to queue families that could be found
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, getContext().surface,
                                         &presentSupport);
    if (presentSupport) {
      indices.presentFamily = i;
    }
    if (indices.isComplete())
      break;
    i++;
  }

  return indices;
}

bool DeviceManager::isDeviceSuitable(VkPhysicalDevice device) {
  QueueFamilyIndices indices =
      findQueueFamilies(device); // Check the queues capabilities
  bool extensionsSupported =
      checkDeviceExtensionSupport(device); // Check the extension support
  bool swapChainAdequate = false;          // Check the swap chain support
  if (extensionsSupported) { // Only if swapChain extension is found
    querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() &&
                        !swapChainSupport.presentModes.empty();
  }
  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void DeviceManager::pickPhysicalDevice() {
  // Holde for the selected device
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  // Retrieve the list of the devices
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(getContext().instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(getContext().instance, &deviceCount,
                             devices.data());
  // Use an ordered map to automatically sort candidates by increasing score
  std::multimap<int, VkPhysicalDevice> candidates;

  for (const auto &device : devices) {
    if (isDeviceSuitable(device))
      candidates.insert(std::make_pair(rateDeviceSuitability(device), device));
  }

  // Check if the best candidate is suitable at all
  if (candidates.rbegin()->first > 0) {
    physicalDevice = candidates.rbegin()->second;
  } else {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
  getContext().physicalDevice = physicalDevice;
}