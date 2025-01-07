#pragma once

#include "api/scene.hh"
#include "vulkan/device/device_manager.hh"
#include "vulkan/extension_manager.hh"
#include "vulkan/gui/gui_manager.hh"
#include "vulkan/rendering/command_pool_manager.hh"
#include "vulkan/swap_chain/surface_manager.hh"
#include "vulkan/swap_chain/swap_chain_manager.hh"
#include "vulkan/window_manager.hh"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <vulkan/vulkan_core.h>

class VulkanApplication {
public:
  VulkanApplication();
  void init();

private:
  WindowManager window_manager;
  ExtensionManager extension_manager;
  SwapChainManager swap_chain_manager;
  DeviceManager device_manager;
  SurfaceManager surface_manager;
  CommandPoolManager command_pool_manager;
  GUIManager gui_manager;

  void createInstance();

  void initVulkan();

  void recreateSwapChain();

  void setupGraphics(Flim::Scene &scene);

  bool mainLoop(const std::function<void(float)> &renderMethod, Flim::Scene &scene);

  void finish();

  void cleanup(Flim::Scene &scene);

  friend class Flim::FlimAPI;
};
