#pragma once

#include "vulkan/base_manager.hh"
#include "vulkan/context.hh"
#include <csignal>
#include <vector>
#include <vulkan/vulkan_core.h>

class ExtensionManager : public BaseManager {
public:
  ExtensionManager(VulkanContext &context) : BaseManager(context) {};
  void cleanUp();
  void listExtensions(); // For debug purposes only
  bool checkValidationLayerSupport();
  void activateDebugExtensions(VkInstanceCreateInfo &info);
  void activateExtensions(VkInstanceCreateInfo &info);
  void setupDebugMessenger();
  void populateDebugMessengerCreateInfo();
  void populateRequiredExtensions();

private:
  static VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
      const VkAllocationCallbacks *pAllocator,
      VkDebugUtilsMessengerEXT *pDebugMessenger);
  static void
  DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                VkDebugUtilsMessengerEXT debugMessenger,
                                const VkAllocationCallbacks *pAllocator);

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  VkDebugUtilsMessengerEXT debugMessenger;
  std::vector<const char *> requiredExtensions;
};
