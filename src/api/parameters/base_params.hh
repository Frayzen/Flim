#pragma once

#include "consts.hh"
#include "vulkan/buffers/attribute_descriptors.hh"
#include "vulkan/buffers/uniform_descriptors.hh"
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vulkan/vulkan_core.h>

class Pipeline;
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

class BaseParams {

public:
  BaseParams(std::string name) : name(name) {};
  std::string name;

  // Uniforms
  GeneralUniDesc &updateUniform(int binding);
  ImageUniDesc &setUniformImage(int binding);
  GeneralUniDesc &setUniform(int binding,
                             int shaderStage = VERTEX_SHADER_STAGE |
                                               FRAGMENT_SHADER_STAGE);
  ImageUniDesc &setUniformImage(int binding, std::string path,
                                int shaderStage = VERTEX_SHADER_STAGE |
                                                  FRAGMENT_SHADER_STAGE);
  void removeUniform(int binding);

  template <typename T>
    requires Derived<T, UniformDescriptor>
  void setUniform(UniformDescriptor &uniform) {
    // no delete as we dont manage the object
    uniforms[uniform.getBinding()] = (std::shared_ptr<UniformDescriptor>(
        &uniform, [](UniformDescriptor *) {}));
  }

  const std::map<int, std::shared_ptr<UniformDescriptor>> &
  getUniformDescriptors() const;

  // Attributes
  AttributeDescriptor &updateAttribute(int binding);
  void removeAttribute(int binding);

  AttributeDescriptor &setAttribute(AttributeDescriptor &attr,
                                    int binding = -1);
  AttributeDescriptor &copyAttribute(int fromBinding, int toBinding);

  const std::map<int, std::shared_ptr<AttributeDescriptor>> &
  getAttributeDescriptors() const;

  virtual bool usable() const;

  std::vector<VkVertexInputBindingDescription> getBindingDescription() const;

  std::vector<VkVertexInputAttributeDescription>
  getAttributeDescriptions() const;

protected:
  std::map<int, std::shared_ptr<UniformDescriptor>> uniforms;
  std::map<int, std::shared_ptr<AttributeDescriptor>> attributes;
  BaseParams() = default;
  BaseParams(const BaseParams &other) : uniforms(), attributes() {
    for (auto &attr : other.attributes) {
      attributes[attr.first] =
          attr.second->clone(); // we update the id of the provided one
    }
    for (auto &uni : other.uniforms) {
      uniforms[uni.first] =
          uni.second->clone(); // we update the id of the provided one
    }
  };
  friend class ::Pipeline;
};

} // namespace Flim
