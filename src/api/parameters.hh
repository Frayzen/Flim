#pragma once

#include "api/shaders/shader.hh"
#include "vulkan/buffers/descriptors.hh"
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
namespace Flim {
enum RenderMode {
  RENDERER_MODE_TRIS = 0,
  RENDERER_MODE_LINE,
  RENDERER_MODE_POINTS,
};

inline VkPolygonMode renderModeToPolygonMode(RenderMode mode) {
  switch (mode) {
  case RENDERER_MODE_LINE:
    return VK_POLYGON_MODE_LINE;
  case RENDERER_MODE_POINTS:
    return VK_POLYGON_MODE_POINT;
  default:
    return VK_POLYGON_MODE_FILL;
  }
}

struct RenderParams {
  Shader vertexShader;
  Shader fragmentShader;
  bool useBackfaceCulling = true;
  int version = 0;
  RenderMode mode = RenderMode::RENDERER_MODE_TRIS;

  std::shared_ptr<GeneralDescriptor> addGeneralDescriptor(int binding) {
    std::shared_ptr<GeneralDescriptor> ptr =
        std::make_unique<GeneralDescriptor>(binding);
    descriptors.push_back(ptr);
    return ptr;
  }

  std::shared_ptr<ImageDescriptor> addImageDescriptor(int binding,
                                                      std::string path) {
    std::shared_ptr<ImageDescriptor> ptr =
        std::make_unique<ImageDescriptor>(binding, path);
    descriptors.push_back(ptr);
    return ptr;
  }

  bool valid() {
    return !vertexShader.code.empty() && !fragmentShader.code.empty();
  }

  void invalidate() { version++; };

  std::vector<std::shared_ptr<BaseDescriptor>> descriptors =
      std::vector<std::shared_ptr<BaseDescriptor>>();
};

struct FlimParameters {};

} // namespace Flim
