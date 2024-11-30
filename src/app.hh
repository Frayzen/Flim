#pragma once

#include "fwd.h"
#include "vulkan/context_holder.hh"
#include "vulkan/device/device_manager.hh"
#include "vulkan/extension_manager.hh"
#include "vulkan/surface_manager.hh"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

class VulkanApplication {
public:
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  ExtensionManager extension_manager;
  DeviceManager device_manager;
  SurfaceManager surface_manager;

  void initWindow() {
    if (!glfwInit()) {
      throw std::runtime_error("Failed to initialize GLFW");
    }
    if (!glfwVulkanSupported()) {
      throw std::runtime_error("Vulkan is not supported by GLFW");
    }
    glfwWindowHint(GLFW_CLIENT_API,
                   GLFW_NO_API);                // Disable OpenGL (default API)
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // For later
    getContext().window =
        glfwCreateWindow(width, height, "Rengine", nullptr, nullptr);
    if (!getContext().window) {
      throw std::runtime_error("Failed to create GLFW window");
    }
    const char *desc;
    if (glfwGetError(&desc)) { // In case an error occured
      throw std::runtime_error(desc);
    }
  }

  void createInstance() {
    if (enableValidationLayers &&
        !extension_manager.checkValidationLayerSupport()) {
      throw std::runtime_error(
          "validation layers requested, but not available!");
    }
    VkApplicationInfo appInfo{}; // used to set pNext to nullptr by default;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Rengine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    extension_manager.activateExtensions(createInfo);
    extension_manager.activateDebugExtensions(createInfo);

    if (vkCreateInstance(&createInfo, nullptr, &getContext().instance) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to create vulkan instance!");
    }
  }

  void initVulkan() {
    /* extension_manager.listExtensions(); */
    createInstance();
    extension_manager.setupDebugMessenger();
    surface_manager.createSurface();
    device_manager.pickPhysicalDevice();
    device_manager.createLogicalDevice();
    device_manager.createSwapChain();
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(getContext().window)) {
      glfwPollEvents();
    }
  }

  void cleanup() {
    vkDestroySwapchainKHR(getContext().device, getContext().swapChain, nullptr);
    vkDestroyDevice(getContext().device, nullptr);
    vkDestroySurfaceKHR(getContext().instance, getContext().surface, nullptr);
    extension_manager.cleanUp();
    vkDestroyInstance(getContext().instance, nullptr);
    glfwDestroyWindow(getContext().window);
    glfwTerminate();
  }
};
