#pragma once

#include <fwd.hh>

class SurfaceManager {
public:
  SurfaceManager() = default;

  void createSurface();

  void setupSwapChainImages();
  void createImageViews();

  void createDepthResources();

private:
};
