#include "device_manager.hh"
#include "vulkan/context_holder.hh"
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
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

bool isDeviceSuitable(VkPhysicalDevice device) {
  QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  return indices.isComplete() && extensionsSupported;
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
    if (checkDeviceExtensionSupport(device))
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
