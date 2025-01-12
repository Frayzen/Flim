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
  assert(attributes.contains(binding) &&
         "You are trying to update a not set attribute");
  attributes[binding] = attributes[binding]->clone();
  return *std::dynamic_pointer_cast<AttributeDescriptor>(attributes[binding]);
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

std::vector<VkVertexInputBindingDescription>
BaseParams::getBindingDescription() const {
  auto nbBindings = attributes.size();
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(nbBindings);
  int i = 0;
  for (auto &attr : attributes)
    bindingDescriptions[i++] = attr.second->getBindingDescription();
  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>
BaseParams::getAttributeDescriptions() const {
  int amount = 0;
  for (auto &d : attributes)
    amount += d.second->getOffsets().size();

  std::vector<VkVertexInputAttributeDescription> attributeDescriptions(amount);

  int cur = 0;
  for (auto &desc : attributes) {
    int amountOffset = desc.second->getOffsets().size();
    for (int i = 0; i < amountOffset; i++) {
      attributeDescriptions[cur + i] = desc.second->getAttributeDesc(i);
      attributeDescriptions[cur + i].location = desc.first + i;
    }
    cur += amountOffset;
  }
  return attributeDescriptions;
}

}; // namespace Flim
