#pragma once

#include "api/scene.hh"
#include "api/tree/instance_object.hh"
#include "api/tree/tree_object.hh"
#include "vulkan/buffers/buffer_manager.hh"
#include "vulkan/context.hh"
#include "vulkan/device/device_manager.hh"
#include "vulkan/extension_manager.hh"
#include "vulkan/rendering/command_pool_manager.hh"
#include "vulkan/rendering/pipeline_manager.hh"
#include "vulkan/swap_chain/surface_manager.hh"
#include "vulkan/swap_chain/swap_chain_manager.hh"
#include "vulkan/window_manager.hh"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

class VulkanApplication {
public:
  VulkanApplication()
      : window_manager(context), extension_manager(context),
        swap_chain_manager(context), device_manager(context),
        surface_manager(context), command_pool_manager(context),
        buffer_manager(context), pipeline_manager(context) {
    context.currentImage = 0;
  }

  void init() {
    window_manager.initWindow();
    initVulkan();
  }

  void run(Flim::Scene &scene) { mainLoop(scene); }

  void setKeyCallback() {}

private:
  VulkanContext context;
  WindowManager window_manager;
  ExtensionManager extension_manager;
  SwapChainManager swap_chain_manager;
  DeviceManager device_manager;
  SurfaceManager surface_manager;
  CommandPoolManager command_pool_manager;
  BufferManager buffer_manager;
  PipelineManager pipeline_manager;

  void createInstance() {
    if (enableValidationLayers &&
        !extension_manager.checkValidationLayerSupport()) {
      throw std::runtime_error(
          "validation layers requested, but not available!");
    }
    VkApplicationInfo appInfo{}; // used to set pNext to nullptr by default;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Flim";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    extension_manager.activateExtensions(createInfo);
    extension_manager.activateDebugExtensions(createInfo);

    if (vkCreateInstance(&createInfo, nullptr, &context.instance) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to create vulkan instance!");
    }
  }

  void initVulkan() {
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
    pipeline_manager.createRenderPass();
    buffer_manager.createDescriptorSetLayout();
    command_pool_manager.createCommandPool();
    command_pool_manager.createCommandBuffers();
    command_pool_manager.createSyncObjects();

    // Frame and depth buffer
    buffer_manager.createDepthResources();
    surface_manager.createFramebuffers();
  }

  void setupGraphics(Flim::Scene &scene) {
    // Graphic pipeline
    pipeline_manager.createGraphicPipeline(*scene.renderer);

    const auto &instance = scene.getRoot().findAny<Flim::InstanceObject>();

    // Texture
    buffer_manager.createTextureImage();
    buffer_manager.createTextureImageView();
    buffer_manager.createTextureSampler();

    // Vertices
    buffer_manager.createVertexBuffer(instance->mesh.getVertices());
    buffer_manager.createIndexBuffer(instance->mesh.getTriangles());

    // Uniform
    buffer_manager.createUniformBuffers();
    buffer_manager.createDescriptorPool();
    buffer_manager.createDescriptorSets();
  }

  void recreateSwapChain() {
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
    surface_manager.createFramebuffers();
    context.currentImage = 0;
  }

  bool mainLoop(Flim::Scene &scene) {
    const auto &instance = scene.getRoot().findAny<Flim::InstanceObject>();
    glfwPollEvents();

    if (command_pool_manager.acquireFrame()) {
      recreateSwapChain();
      return false;
    }
    buffer_manager.updateUniformBuffer(instance->mesh, scene.mainCamera);
    if (command_pool_manager.renderFrame(
            window_manager.framebufferResized,
            instance->mesh.getTriangles().size())) {
      window_manager.framebufferResized = false;
      recreateSwapChain();
    }
    return glfwWindowShouldClose(context.window);
  }

  void finish() { vkDeviceWaitIdle(context.device); }

  void cleanup() {
    swap_chain_manager.cleanup();
    buffer_manager.cleanup();
    pipeline_manager.cleanup();
    command_pool_manager.cleanup();
    vkDestroyDevice(context.device, nullptr);
    extension_manager.cleanUp();
    vkDestroySurfaceKHR(context.instance, context.surface, nullptr);
    vkDestroyInstance(context.instance, nullptr);
    glfwDestroyWindow(context.window);
    glfwTerminate();
  }

  friend class Flim::FlimAPI;
};
