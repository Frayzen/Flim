#include "attribute_descriptors.hh"
#include "api/render/mesh.hh"
#include "fwd.hh"
#include "vulkan/buffers/buffer_utils.hh"
#include "vulkan/context.hh"
#include "vulkan/rendering/renderer.hh"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
namespace Flim {

AttributeDescriptor::AttributeDescriptor(int binding, AttributeRate rate)
    : BaseAttributeDescriptor(binding), rate(rate), size(0) {};

AttributeDescriptor &AttributeDescriptor::add(long offset, VkFormat format) {
  offsets.push_back(std::make_pair(offset, format));
  return *this;
}

int AttributeDescriptor::getAmountOffset() const { return offsets.size(); }

VkVertexInputAttributeDescription
AttributeDescriptor::getAttributeDesc(int id) const {
  assert((unsigned long)id < offsets.size());
  VkVertexInputAttributeDescription desc{};
  desc.binding = binding;
  desc.location = id;
  desc.offset = offsets[id].first;
  desc.format = offsets[id].second;
  return desc;
}

VkVertexInputBindingDescription
AttributeDescriptor::getBindingDescription() const {
  assert(size != 0 && "please populate the attribute descriptor");
  VkVertexInputBindingDescription desc{};
  desc.binding = binding;
  desc.stride = size;
  desc.inputRate = attributeRateToInputRate(rate);
  return desc;
}

inline int getElemSize(AttributeRate &rate, Renderer &r) {
  switch (rate) {
  case AttributeRate::INSTANCE:
    return r.getInstances().size();
  case AttributeRate::VERTEX:
    return r.mesh.vertices.size();
  }
  throw std::runtime_error("Unexpected attribute rate");
  return 0;
}

void AttributeDescriptor::setup(Renderer &renderer) {
  VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (isComputeFriendly) {
    assert(
        isOnlySetup &&
        "If the attribute is compute friendly, it also need to be only setup");
    usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }
  assert(size != 0 && "please populate the attribute descriptor");
  Flim::Mesh &mesh = renderer.mesh;
  size_t bufSize = getElemSize(rate, renderer) * size;
  auto &buffers = renderer.attributes[id];
  auto &datas = renderer.mappedAttributes[id];
  for (int i = 0; i < redudancyAmount(); i++) {
    Buffer &buffer = buffers.emplace_back();
    void *&data = datas.emplace_back(nullptr);
    if (isOnlySetup) {
      void *tmp = malloc(bufSize);
      updateFunction(renderer.mesh, tmp);
      createBufferFromData(buffer, usage, tmp, bufSize);
      free(tmp);
      data = nullptr;
    } else
      data = createMappedBuffer(buffer, usage, bufSize);
  }
}

AttributeDescriptor &AttributeDescriptor::computeFriendly(bool val) {
  isComputeFriendly = val;
  return *this;
}

AttributeDescriptor &AttributeDescriptor::onlySetup(bool val) {
  isOnlySetup = val;
  return *this;
}

void AttributeDescriptor::update(Renderer &renderer) {
  if (isOnlySetup)
    return;
  int cur = context.currentImage % redudancyAmount();
  updateFunction(renderer.mesh, renderer.mappedAttributes[id][cur]);
}

const Buffer &AttributeDescriptor::getBuffer(const Renderer &renderer,
                                             int currentImage) const {
  if (usesPreviousFrame)
    currentImage--;
  currentImage = std::clamp(currentImage, 0, redudancyAmount());
  auto &buffers = renderer.attributes.find(id)->second;
  return buffers[currentImage];
}

void AttributeDescriptor::cleanup(Renderer &renderer) {
  auto &buffers = renderer.attributes[id];
  auto &mapped = renderer.mappedAttributes[id];
  for (int i = 0; i < redudancyAmount(); i++) {
    if (!isOnlySetup)
      vkUnmapMemory(context.device, buffers[i].bufferMemory);
    vkDestroyBuffer(context.device, buffers[i].buffer, nullptr);
    vkFreeMemory(context.device, buffers[i].bufferMemory, nullptr);
  }
}

} // namespace Flim
