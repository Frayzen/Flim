#pragma once
#include "vulkan/graphics/pipeline.hh"
#include <fwd.h>

class SurfaceManager {
public:
  void createSurface();
  void setupSwapChainImages();
  void createImageViews();
  void createFramebuffers();
  void cleanup();

private:
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  friend Pipeline;
};
