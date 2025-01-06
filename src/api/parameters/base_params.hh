#pragma once

#include "consts.hh"
#include "vulkan/buffers/attribute_descriptors.hh"
#include "vulkan/buffers/uniform_descriptors.hh"
#include <map>
#include <memory>
#include <string>
#include <type_traits>
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
  AttributeDescriptor &setAttribute(int binding,
                                    AttributeRate rate = AttributeRate::VERTEX);
  void removeAttribute(int binding);

  template <typename T>
    requires Derived<T, BaseAttributeDescriptor>
  T &setAttribute(T &attr, int binding = -1) {
    if (binding == -1)
      binding = attr.binding;
    std::shared_ptr<BaseAttributeDescriptor> cloned = attr.clone();
    cloned->binding = binding;
    attributes[binding] = cloned;
    return *std::dynamic_pointer_cast<T>(attributes[binding]);
  }
  AttributeDescriptor &copyAttribute(int fromBinding, int toBinding);

  const std::map<int, std::shared_ptr<BaseAttributeDescriptor>> &
  getAttributeDescriptors() const;

  virtual bool usable() const;

protected:
  std::map<int, std::shared_ptr<UniformDescriptor>> uniforms;
  std::map<int, std::shared_ptr<BaseAttributeDescriptor>> attributes;

  friend class ::Pipeline;
};

} // namespace Flim
