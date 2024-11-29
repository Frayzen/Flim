#pragma once

#include <csignal>
#include <vector>
#include <vulkan/vulkan_core.h>

class ExtensionManager {
public:
  void cleanUp();
  void listExtensions(); // For debug purposes only
  bool checkValidationLayerSupport();
  void activateDebugExtensions(VkInstanceCreateInfo &info);
  void activateExtensions(VkInstanceCreateInfo &info);
  void setupDebugMessenger();
  void populateDebugMessengerCreateInfo();
  void populateRequiredExtensions();

private:
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  VkDebugUtilsMessengerEXT debugMessenger;
  std::vector<const char *> requiredExtensions;
};
