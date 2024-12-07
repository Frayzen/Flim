#include "device_manager.hh"
#include "vulkan/context.hh"
#include "vulkan/device/device_utils.hh"
#include "vulkan/swap_chain/swap_chain_utils.hh"

#include <cstdint>
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

bool DeviceManager::isDeviceSuitable(VkPhysicalDevice device) {
  QueueFamilyIndices indices =
      findQueueFamilies(context, device); // Check the queues capabilities
  bool extensionsSupported =
      checkDeviceExtensionSupport(device); // Check the extension support
  bool swapChainAdequate = false;          // Check the swap chain support
  if (extensionsSupported) { // Only if swapChain extension is found
    SwapChainSupportDetails swapChainSupport =
        querySwapChainSupport(context, device);
    swapChainAdequate = !swapChainSupport.formats.empty() &&
                        !swapChainSupport.presentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
  return indices.isComplete() && extensionsSupported && swapChainAdequate &&
         supportedFeatures.samplerAnisotropy;
}

void DeviceManager::pickPhysicalDevice() {
  // Holde for the selected device
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  // Retrieve the list of the devices
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(context.instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(context.instance, &deviceCount, devices.data());
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
  context.physicalDevice = physicalDevice;
}
