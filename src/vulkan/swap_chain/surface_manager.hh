#pragma once

#include "vulkan/base_manager.hh"
#include "vulkan/context.hh"
#include <fwd.h>

class SurfaceManager : BaseManager {
public:
  SurfaceManager(VulkanContext &context) : BaseManager(context) {};

  void createSurface();

  void setupSwapChainImages();
  void createImageViews();
  void createFramebuffers();
};
