#pragma once

#include "api/shaders/shader.hh"
#include "vulkan/buffers/descriptors.hh"
#include <iostream>
#include <memory>
#include <vector>
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

typedef std::vector<std::shared_ptr<BaseDescriptor>> DescriptorList;
struct Renderer {
  Shader vertexShader;
  Shader fragmentShader;
  RendererMode mode = RendererMode::RENDERER_MODE_TRIS;

  std::shared_ptr<GeneralDescriptor> addGeneralDescriptor(int binding) {
    std::shared_ptr<GeneralDescriptor> ptr =
        std::make_unique<GeneralDescriptor>(binding);
    descriptors.push_back(ptr);
    return ptr;
  }

  bool valid() {
    return !vertexShader.code.empty() && !fragmentShader.code.empty();
  }

  DescriptorList descriptors = DescriptorList();
};

struct FlimParameters {};

} // namespace Flim
