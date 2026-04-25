#pragma once

#include <fwd.hh>

namespace Flim {

class SurfaceManager {
public:
  SurfaceManager() = default;

  void createSurface();

  void setupSwapChainImages();
  void createImageViews();

  void createDepthResources();

private:
};
} // namespace Flim
