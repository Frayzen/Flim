#pragma once

#include "api/shaders/shader.hh"
#include <vulkan/vulkan_core.h>
namespace Flim {
enum RendererMode {
  RENDERER_MODE_TRIS = 0,
  RENDERER_MODE_LINE,
  RENDERER_MODE_POINTS,
};

inline VkPolygonMode renderModeToPolygonMode(RendererMode mode) {
  switch (mode) {
  case RENDERER_MODE_LINE:
    return VK_POLYGON_MODE_LINE;
  case RENDERER_MODE_POINTS:
    return VK_POLYGON_MODE_POINT;
  default:
    return VK_POLYGON_MODE_FILL;
  }
}

struct Renderer {
  Shader vertexShader;
  Shader fragmentShader;
  RendererMode mode = RendererMode::RENDERER_MODE_TRIS;
};

struct FlimParameters {};

} // namespace Flim
