#include "app.hh"
#include "api/scene.hh"
#include <chrono>
#include <iostream>
#include <stdexcept>

VulkanContext context{};

VulkanApplication::VulkanApplication()
    : window_manager(), extension_manager(), swap_chain_manager(),
      device_manager(), surface_manager(), command_pool_manager(),
      buffer_manager(), gui_manager() {
  context.currentImage = 0;
  context.currentUpdate = 1;
}

void VulkanApplication::init() {
  window_manager.initWindow();
  initVulkan();
}

void VulkanApplication::createInstance() {
  if (enableValidationLayers &&
      !extension_manager.checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }
  VkApplicationInfo appInfo{}; // used to set pNext to nullptr by default;
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Flim";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_MAKE_VERSION(1, 3, 0);

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  extension_manager.activateExtensions(createInfo);
  extension_manager.activateDebugExtensions(createInfo);

  if (vkCreateInstance(&createInfo, nullptr, &context.instance) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create vulkan instance!");
  }
  std::cout << "vulkan initialized" << std::endl;
}

void VulkanApplication::initVulkan() {
  // Setup
  createInstance();
  extension_manager.setupDebugMessenger();
  surface_manager.createSurface();

  // Device
  device_manager.pickPhysicalDevice();
  device_manager.createLogicalDevice();

  // Swap chain
  swap_chain_manager.createSwapChain();
  surface_manager.setupSwapChainImages();
  surface_manager.createImageViews();

  // Create command pool
  command_pool_manager.createCommandPool();
  command_pool_manager.createCommandBuffers();
  command_pool_manager.createSyncObjects();

  // Frame and depth buffer
  buffer_manager.createDepthResources();
  gui_manager.setup();
}

void VulkanApplication::setupGraphics(Flim::Scene &scene) {
  for (auto &r : scene.renderers)
    r.second->setup();
  for (auto &c : scene.computers)
    c.second->setup();
}

void VulkanApplication::recreateSwapChain() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(context.window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(context.window, &width, &height);
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(context.device);
  swap_chain_manager.cleanup();
  swap_chain_manager.createSwapChain();
  surface_manager.setupSwapChainImages();
  surface_manager.createImageViews();
  buffer_manager.createDepthResources();
  context.currentImage = 0;
}

static std::chrono::high_resolution_clock timer;
static auto previous = timer.now();

bool VulkanApplication::mainLoop(const std::function<void(float)> &renderMethod,
                                 Flim::Scene &scene) {
  const auto &camera = scene.camera;
  glfwPollEvents();

  if (command_pool_manager.acquireFrame()) {
    recreateSwapChain();
    return false;
  }

  for (auto &r : scene.renderers)
    r.second->update(camera);

  for (auto computer : scene.computers) {
    command_pool_manager.recordCommandBuffer(*computer.second);
  }

  for (auto renderer : scene.renderers) {
    command_pool_manager.recordCommandBuffer(*renderer.second);
  }

  // Assuming timer and previous are already defined.
  auto now = timer.now();
  // Automatically in seconds as float
  std::chrono::duration<float> deltaTime = now - previous;
  previous = now;

  float deltaSeconds = deltaTime.count(); // Convert to float for easier use

  static float deltatime;
  gui_manager.beginFrame();
  renderMethod(deltaTime.count());
  gui_manager.endFrame();
  if (command_pool_manager.submitFrame(window_manager.framebufferResized)) {
    window_manager.framebufferResized = false;
    recreateSwapChain();
  }
  return glfwWindowShouldClose(context.window);
}

void VulkanApplication::finish() { vkDeviceWaitIdle(context.device); }

void VulkanApplication::cleanup(Flim::Scene &scene) {
  gui_manager.cleanup();
  swap_chain_manager.cleanup();
  for (auto &r : scene.renderers) {
    r.second->cleanup();
  }
  for (auto &c : scene.computers) {
    c.second->cleanup();
  }

  /* pipeline_manager.cleanup(); */
  command_pool_manager.cleanup();
  vkDestroyDevice(context.device, nullptr);
  extension_manager.cleanUp();
  vkDestroySurfaceKHR(context.instance, context.surface, nullptr);
  vkDestroyInstance(context.instance, nullptr);
  glfwDestroyWindow(context.window);
  glfwTerminate();
}
