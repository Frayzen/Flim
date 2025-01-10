#include "vulkan/buffers/attribute_descriptors.hh"
#include "api/render/mesh.hh"
#include "vulkan/buffers/descriptor_holder.hh"
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
namespace Flim {

AttributeDescriptor::AttributeDescriptor(Mesh &mesh, int binding,
                                         AttributeRate rate)
    : BaseAttributeDescriptor(mesh, binding), rate(rate), size(0),
      updateFunction(nullptr) {};

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

inline int getAmount(const Mesh &m, AttributeRate &rate) {
  switch (rate) {
  case AttributeRate::INSTANCE:
    return m.instances.size();
  case AttributeRate::VERTEX:
    return m.vertices.size();
  }
  throw std::runtime_error("Unexpected attribute rate");
  return 0;
}

void AttributeDescriptor::setup() {
  VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (isOnlySetup)
    usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if (isComputeFriendly) {
    assert(
        isOnlySetup &&
        "If the attribute is compute friendly, it also need to be only setup");
    usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }
  assert(size != 0 && "please populate the attribute descriptor");
  size_t bufSize = getAmount(*mesh, rate) * size;
  static auto memProp = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  setupBuffers(bufSize, usage, memProp);

  for (auto b : getBuffers()) {
    if (isOnlySetup) {
      void *tmp = malloc(bufSize);
      updateFunction(*mesh, tmp);
      b->populate(tmp);
      free(tmp);
    } else
      b->map();
  }
}

VkWriteDescriptorSet
AttributeDescriptor::getDescriptor(DescriptorHolder &holder, int i) {
  static VkWriteDescriptorSet descriptor{};
  descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor.dstSet = holder.descriptorSets[i];
  descriptor.dstBinding = binding;
  descriptor.dstArrayElement = 0;
  descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptor.descriptorCount = 1;
  // pBufferInfo
  int offset = usesPreviousFrame ? -1 : 0;
  static VkDescriptorBufferInfo storageBufferInfoLastFrame{};
  storageBufferInfoLastFrame.buffer =
      getBuffer(i + offset + redundancy)->getVkBuffer();
  storageBufferInfoLastFrame.offset = 0;
  storageBufferInfoLastFrame.range = size * getAmount(*mesh, rate);
  descriptor.pBufferInfo = &storageBufferInfoLastFrame;
  return descriptor;
}

AttributeDescriptor &AttributeDescriptor::computeFriendly(bool val) {
  isComputeFriendly = val;
  return *this;
}

AttributeDescriptor &AttributeDescriptor::onlySetup(bool val) {
  isOnlySetup = val;
  redundancy = 1;
  return *this;
}

void AttributeDescriptor::update() {
  if (isOnlySetup)
    return;
  updateFunction(*mesh, getBuffer()->getPtr());
}

void AttributeDescriptor::cleanup() { cleanupBuffers(); }

} // namespace Flim
