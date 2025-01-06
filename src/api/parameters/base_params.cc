#include "base_params.hh"
namespace Flim {

// Uniforms
GeneralUniDesc &BaseParams::updateUniform(int binding) {
  uniforms[binding] = uniforms[binding]->clone();
  return *std::dynamic_pointer_cast<GeneralUniDesc>(uniforms[binding]);
}
ImageUniDesc &BaseParams::setUniformImage(int binding) {
  uniforms[binding] = uniforms[binding]->clone();
  return *std::dynamic_pointer_cast<ImageUniDesc>(uniforms[binding]);
}
GeneralUniDesc &BaseParams::setUniform(int binding, int shaderStage) {
  std::shared_ptr<GeneralUniDesc> ptr =
      std::make_shared<GeneralUniDesc>(binding, shaderStage);
  uniforms[binding] = ptr;
  return *ptr;
}
ImageUniDesc &BaseParams::setUniformImage(int binding, std::string path,
                                          int shaderStage) {
  std::shared_ptr<ImageUniDesc> ptr =
      std::make_shared<ImageUniDesc>(binding, path, shaderStage);
  uniforms[binding] = ptr;
  return *ptr;
}
void BaseParams::removeUniform(int binding) { uniforms.erase(binding); }

const std::map<int, std::shared_ptr<UniformDescriptor>> &
BaseParams::getUniformDescriptors() const {
  return uniforms;
}

// Attributes
AttributeDescriptor &BaseParams::updateAttribute(int binding) {
  attributes[binding] = attributes[binding]->clone();
  return *std::dynamic_pointer_cast<AttributeDescriptor>(attributes[binding]);
}
AttributeDescriptor &BaseParams::setAttribute(int binding, AttributeRate rate) {
  std::shared_ptr<AttributeDescriptor> ptr =
      std::make_shared<AttributeDescriptor>(binding, rate);
  attributes[binding] = ptr;
  return *ptr;
}
void BaseParams::removeAttribute(int binding) { attributes.erase(binding); }
const std::map<int, std::shared_ptr<BaseAttributeDescriptor>> &
BaseParams::getAttributeDescriptors() const {
  return attributes;
}

AttributeDescriptor &BaseParams::copyAttribute(int fromBinding, int toBinding) {
  assert(attributes.contains(fromBinding));
  attributes[toBinding] = attributes[fromBinding]->clone();
  return *std::dynamic_pointer_cast<AttributeDescriptor>(
      attributes[fromBinding]);
}

bool BaseParams::usable() const { return true; }
}; // namespace Flim
