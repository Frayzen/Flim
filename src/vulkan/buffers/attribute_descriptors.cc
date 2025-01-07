#include "attribute_descriptors.hh"
#include "api/render/mesh.hh"
#include "vulkan/buffers/buffer_utils.hh"
#include "vulkan/buffers/descriptor_holder.hh"
#include "vulkan/context.hh"
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
  if (isComputeFriendly) {
    assert(
        isOnlySetup &&
        "If the attribute is compute friendly, it also need to be only setup");
    usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }
  assert(size != 0 && "please populate the attribute descriptor");
  const Flim::Mesh &mesh = *context.rctx.mesh;
  size_t bufSize = getAmount(mesh, rate) * size;
  static auto memProp = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  setupBuffers(bufSize, usage, memProp);

  for (auto &b : getBuffers()) {
    if (isOnlySetup) {
      void *tmp = malloc(bufSize);
      updateFunction(mesh, tmp);
      b.populate(tmp);
      free(tmp);
    } else
      b.map();
  }
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
  updateFunction(*context.rctx.mesh, getBuffer().getPtr());
}

void AttributeDescriptor::cleanup() { cleanupBuffers(); }

} // namespace Flim
