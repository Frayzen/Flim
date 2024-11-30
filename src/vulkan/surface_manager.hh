#pragma once
#include <fwd.h>

class SurfaceManager {
public:
  void createSurface();
  void setupSwapChainImages();
  void createImageViews();
  void cleanup();

private:
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;
};
