#pragma once

#include <fwd.hh>

class SurfaceManager {
public:
  SurfaceManager() {};

  void createSurface();

  void setupSwapChainImages();
  void createImageViews();
};
