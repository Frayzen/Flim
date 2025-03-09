#include "vulkan/buffers/attribute_descriptors.hh"
#include "api/render/mesh.hh"
#include "utils/checks.hh"
#include "vulkan/buffers/descriptor_holder.hh"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
namespace Flim {

AttributeDescriptor::AttributeDescriptor(int binding, AttributeRate rate)
    : BufferHolder(), binding(binding), usesPreviousFrame(false), rate(rate),
      size(0), updateFunction(nullptr), isSingleBuffered(false),
      isComputeFriendly(false), isOnlySetup(false) {};

AttributeDescriptor &AttributeDescriptor::add(long offset, VkFormat format) {
  offsets.push_back(std::make_pair(offset, format));
  return *this;
}

std::vector<VkDeviceSize> AttributeDescriptor::getOffsets() const {
  std::vector<VkDeviceSize> ret(offsets.size());
  for (size_t i = 0; i < offsets.size(); i++)
    ret[i] = offsets[i].first;
  return ret;
}

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
  CHECK(size != 0, "Please populate the attribute descriptor");
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
  CHECK(!offsets.empty(), "Please specify the offsets of the attribute");
  CHECK(size != 0, "please populate the attribute descriptor");
  assert(
      getAttachedMesh() != nullptr &&
      "You cannot use an attribute without registering it"); // should be set by
                                                             // the renderer
  if (isSingleBuffered)
    redundancy = 1;
  VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (isOnlySetup)
    usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if (isComputeFriendly) {
    assert(
        isOnlySetup &&
        "If the attribute is compute friendly, it also need to be only setup");
    usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }
  size_t bufSize = getAmount(*getAttachedMesh(), rate) * size;
  static auto memProp = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  setupBuffers(bufSize, usage, memProp, isComputeFriendly);

  for (auto b : getBuffers()) {
    if (isOnlySetup) {
      void *tmp = malloc(bufSize);
      updateFunction(*getAttachedMesh(), tmp);
      b->populate(tmp);
      free(tmp);
    } else
      b->map();
  }
}

VkWriteDescriptorSet
AttributeDescriptor::getDescriptor(DescriptorHolder &holder, int i) {
  VkWriteDescriptorSet descriptor{};
  descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptor.dstSet = holder.descriptorSets[i];
  descriptor.dstBinding = binding;
  descriptor.dstArrayElement = 0;
  descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptor.descriptorCount = 1;
  // pBufferInfo
  int offset = usesPreviousFrame ? -1 : 0;
  storageBufferInfo = {};
  auto buf = getBuffer(i + offset + redundancy);
  storageBufferInfo.buffer = buf->getVkBuffer();
  storageBufferInfo.offset = 0;
  storageBufferInfo.range = buf->getSize();
  descriptor.pBufferInfo = &storageBufferInfo;
  return descriptor;
}

AttributeDescriptor &AttributeDescriptor::computeFriendly(bool val) {
  isComputeFriendly = val;
  return *this;
}

AttributeDescriptor &AttributeDescriptor::onlySetup(bool val) {
  isOnlySetup = val;
  return *this;
}

AttributeDescriptor &AttributeDescriptor::singleBuffered(bool val) {
  isSingleBuffered = val;
  return *this;
}

void AttributeDescriptor::update() {
  if (isOnlySetup)
    return;
  updateFunction(*getAttachedMesh(), getBuffer()->getPtr());
}

void AttributeDescriptor::cleanup() { cleanupBuffers(); }

} // namespace Flim
