#include "app.hh"
#include "api/tree/instance_object.hh"
#include "api/tree/tree_object.hh"
#include <iostream>
#include <stdexcept>

VulkanContext context{};

VulkanApplication::VulkanApplication()
    : window_manager(), extension_manager(), swap_chain_manager(),
      device_manager(), surface_manager(), command_pool_manager(),
      buffer_manager(), pipeline_manager(), descriptors_manager(),
      gui_manager() {
  context.currentImage = 0;
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

void VulkanApplication::updateGraphics(Flim::Scene &scene, bool setup) {
  context.renderer = scene.renderer;
  if (!setup) {
    vkDeviceWaitIdle(context.device);
    pipeline_manager.cleanup();
    vkDestroyDescriptorSetLayout(context.device, context.descriptorSetLayout,
                                 nullptr);
  }
  descriptors_manager.createDescriptorSetLayout();
  pipeline_manager.createGraphicPipeline();
}

void VulkanApplication::setupGraphics(Flim::Scene &scene) {
  updateGraphics(scene, true);

  const auto &instance = scene.getRoot().findAny<Flim::InstanceObject>();

  // Vertices
  buffer_manager.createVertexBuffer(instance->mesh.getVertices());
  buffer_manager.createIndexBuffer(instance->mesh.getTriangles());

  descriptors_manager.setupUniforms();
  descriptors_manager.createDescriptorPool();
  descriptors_manager.createDescriptorSets();
  /*   // Texture */
  /*   buffer_manager.createTextureImage(instance->mesh.getMaterial().texturePath);
   */
  /*   buffer_manager.createTextureImageView(); */
  /*   buffer_manager.createTextureSampler(); */

  /*   // Uniform */
  /*   buffer_manager.createUniformBuffers(); */
  /*   buffer_manager.createDescriptorPool(); */
  /*   buffer_manager.createDescriptorSets(); */
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

bool VulkanApplication::mainLoop(const std::function<void()> &renderMethod,
                                 Flim::Scene &scene) {
  const auto &instance = *scene.getRoot().findAny<Flim::InstanceObject>();
  const auto &camera = *scene.mainCamera;
  glfwPollEvents();

  if (command_pool_manager.acquireFrame()) {
    recreateSwapChain();
    return false;
  }
  descriptors_manager.updateUniforms(instance, camera);
  /* buffer_manager.updateUniformBuffer(instance->mesh, scene.mainCamera); */
  command_pool_manager.renderFrame(instance.mesh.getTriangles().size());
  gui_manager.beginFrame();
  renderMethod();
  gui_manager.endFrame();
  if (command_pool_manager.submitFrame(window_manager.framebufferResized)) {
    window_manager.framebufferResized = false;
    recreateSwapChain();
  }
  return glfwWindowShouldClose(context.window);
}

void VulkanApplication::finish() { vkDeviceWaitIdle(context.device); }

void VulkanApplication::cleanup() {
  descriptors_manager.cleanup();
  gui_manager.cleanup();
  swap_chain_manager.cleanup();
  pipeline_manager.cleanup();
  command_pool_manager.cleanup();
  vkDestroyDevice(context.device, nullptr);
  extension_manager.cleanUp();
  vkDestroySurfaceKHR(context.instance, context.surface, nullptr);
  vkDestroyInstance(context.instance, nullptr);
  glfwDestroyWindow(context.window);
  glfwTerminate();
}
