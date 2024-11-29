#include "extension_manager.hh"

#include "consts.hh"
#include "fwd.h"
#include "vulkan/context_holder.hh"
#include "vulkan/proxy.hh"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>

void ExtensionManager::populateRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  const char *desc;
  if (glfwGetError(&desc)) {
    throw std::runtime_error(desc);
  }

#ifndef NDEBUG
  std::cout << "Required by GLFW (" << glfwExtensionCount << "):" << std::endl;
  for (uint32_t i = 0; i < glfwExtensionCount; i++)
    std::cout << "        " << glfwExtensions[i] << std::endl;
#endif
  for (uint32_t i = 0; i < glfwExtensionCount; i++) {
    requiredExtensions.push_back(glfwExtensions[i]);
  }
  if (enableValidationLayers) {
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
}

void ExtensionManager::cleanUp() {
  if (enableValidationLayers) {
    VulkanProxy::DestroyDebugUtilsMessengerEXT(getContext().instance,
                                               debugMessenger, nullptr);
  }
}

void ExtensionManager::listExtensions() // For debug purposes only
{
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                         extensions.data());
  std::cout << "available extensions:\n";

  for (const auto &extension : extensions) {
    std::cout << '\t' << extension.extensionName << '\n';
  }
}

bool ExtensionManager::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      listExtensions();
      std::cerr << "validation layer '" << layerName << "' not found"
                << std::endl;
      return false;
    }
  }

  return true;
}

void ExtensionManager::activateExtensions(VkInstanceCreateInfo &createInfo) {

  populateRequiredExtensions();
  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(requiredExtensions.size());
  createInfo.ppEnabledExtensionNames = requiredExtensions.data();
}

void ExtensionManager::activateDebugExtensions(
    VkInstanceCreateInfo &createInfo) {

  if (!enableValidationLayers) {
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.pNext = nullptr;
    return;
  }

  // In order to debug the creation of the VkInstance
  populateDebugMessengerCreateInfo();
  createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;

  // Validation layers are modules in charge of checking the values used on
  // vulkan API methods' call It is intended to be used in debug mode only
  createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
  createInfo.ppEnabledLayerNames = validationLayers.data();
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {

  if (messageSeverity >= VK_DEBUG_LEVEL) {
    // Message is important enough to show
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  }

  return VK_FALSE;
}

void ExtensionManager::populateDebugMessengerCreateInfo() {
  debugCreateInfo = {};
  debugCreateInfo.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debugCreateInfo.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debugCreateInfo.pfnUserCallback = debugCallback;
}

void ExtensionManager::setupDebugMessenger() {
  if (!enableValidationLayers)
    return;
  populateDebugMessengerCreateInfo();
  if (VulkanProxy::CreateDebugUtilsMessengerEXT(
          getContext().instance, &debugCreateInfo, nullptr, &debugMessenger) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}
