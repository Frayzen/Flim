#pragma once

#include "api/shaders/shader.hh"
#include "vulkan/buffers/attribute_descriptors.hh"
#include "vulkan/buffers/uniform_descriptors.hh"
#include <map>
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
  RenderParams() = default;

  Shader vertexShader;
  Shader fragmentShader;

  bool useBackfaceCulling = true;
  int version = 0;
  RenderMode mode = RenderMode::RENDERER_MODE_TRIS;

  GeneralUniDesc &addUniform(int binding) {
    std::shared_ptr<GeneralUniDesc> ptr =
        std::make_shared<GeneralUniDesc>(binding);
    uniforms[binding] = ptr;
    return *ptr;
  }

  ImageUniDesc &addUniformImage(int binding, std::string path) {
    std::shared_ptr<ImageUniDesc> ptr =
        std::make_shared<ImageUniDesc>(binding, path);
    uniforms[binding] = ptr;
    return *ptr;
  }

  AttributeDescriptor &
  setAttribute(int binding, AttributeRate rate = AttributeRate::VERTEX) {
    std::shared_ptr<AttributeDescriptor> ptr =
        std::make_shared<AttributeDescriptor>(binding, rate);
    attributes[binding] = ptr;
    return *ptr;
  }

  // Get uniforms
  GeneralUniDesc &getUniform(int binding) {
    uniforms[binding] = uniforms[binding]->clone();
    return *std::dynamic_pointer_cast<GeneralUniDesc>(uniforms[binding]);
  }

  ImageUniDesc &getUniformImage(int binding) {
    uniforms[binding] = uniforms[binding]->clone();
    return *std::dynamic_pointer_cast<ImageUniDesc>(uniforms[binding]);
  }

  // Get attributes
  AttributeDescriptor &getAttribute(int binding) {
    attributes[binding] = attributes[binding]->clone();
    return *std::dynamic_pointer_cast<AttributeDescriptor>(attributes[binding]);
  }

  bool valid() {
    return !vertexShader.code.empty() && !fragmentShader.code.empty();
  }

  void invalidate() { version++; };

  std::map<int, std::shared_ptr<UniformDescriptor>> &getUniformDescriptors() {
    return uniforms;
  }

  std::map<int, std::shared_ptr<BaseAttributeDescriptor>> &
  getAttributeDescriptors() {
    return attributes;
  }

protected:
  std::map<int, std::shared_ptr<UniformDescriptor>> uniforms;
  std::map<int, std::shared_ptr<BaseAttributeDescriptor>> attributes;
};

struct FlimParameters {};

} // namespace Flim
