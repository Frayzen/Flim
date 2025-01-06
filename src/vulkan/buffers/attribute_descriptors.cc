#include "attribute_descriptors.hh"
#include "api/render/mesh.hh"
#include "fwd.hh"
#include "vulkan/buffers/buffer_utils.hh"
#include "vulkan/buffers/descriptor_holder.hh"
#include "vulkan/context.hh"
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

inline int getElemSize(AttributeRate &rate, DescriptorHolder &r) {
  switch (rate) {
  case AttributeRate::INSTANCE:
    return r.mesh.instances.size();
  case AttributeRate::VERTEX:
    return r.mesh.vertices.size();
  }
  throw std::runtime_error("Unexpected attribute rate");
  return 0;
}

void AttributeDescriptor::setup(DescriptorHolder &holder) {
  VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (isComputeFriendly) {
    assert(
        isOnlySetup &&
        "If the attribute is compute friendly, it also need to be only setup");
    usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }
  assert(size != 0 && "please populate the attribute descriptor");
  const Flim::Mesh &mesh = holder.mesh;
  size_t bufSize = getElemSize(rate, holder) * size;
  auto &buffers = holder.attributes[id];
  auto &datas = holder.mappedAttributes[id];
  for (int i = 0; i < redudancyAmount(); i++) {
    Buffer &buffer = buffers.emplace_back();
    void *&data = datas.emplace_back(nullptr);
    if (isOnlySetup) {
      void *tmp = malloc(bufSize);
      updateFunction(holder.mesh, tmp);
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

void AttributeDescriptor::update(DescriptorHolder &holder) {
  if (isOnlySetup)
    return;
  int cur = context.currentImage % redudancyAmount();
  updateFunction(holder.mesh, holder.mappedAttributes[id][cur]);
}

const Buffer &AttributeDescriptor::getBuffer(const DescriptorHolder &holder,
                                             int currentImage) const {
  if (usesPreviousFrame)
    currentImage--;
  currentImage = std::clamp(currentImage, 0, redudancyAmount());
  auto &buffers = holder.attributes.find(id)->second;
  return buffers[currentImage];
}

void AttributeDescriptor::cleanup(DescriptorHolder &holder) {
  auto &buffers = holder.attributes[id];
  auto &mapped = holder.mappedAttributes[id];
  for (int i = 0; i < redudancyAmount(); i++) {
    if (!isOnlySetup)
      vkUnmapMemory(context.device, buffers[i].bufferMemory);
    vkDestroyBuffer(context.device, buffers[i].buffer, nullptr);
    vkFreeMemory(context.device, buffers[i].bufferMemory, nullptr);
  }
}

} // namespace Flim
