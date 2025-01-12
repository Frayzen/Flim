#pragma once

#include "vulkan/buffers/buffer_manager.hh"
#include <Eigen/Core>
#include <functional>
#include <fwd.hh>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

class DescriptorHolder;
class Renderer;
namespace Flim {

class Mesh;

enum AttributeRate {
  VERTEX,
  INSTANCE,
};

inline VkVertexInputRate attributeRateToInputRate(AttributeRate rate) {
  switch (rate) {
  case VERTEX:
    return VK_VERTEX_INPUT_RATE_VERTEX;

  case INSTANCE:
    return VK_VERTEX_INPUT_RATE_INSTANCE;
  }
  throw std::runtime_error("Invalide attribute rate");
  return VK_VERTEX_INPUT_RATE_VERTEX;
}

class AttributeDescriptor : public BufferHolder {

public:
  AttributeDescriptor(int binding, AttributeRate rate);

  AttributeDescriptor &add(long offset, VkFormat format);

  std::vector<VkDeviceSize> getOffsets() const;
  VkVertexInputAttributeDescription getAttributeDesc(int id) const;
  VkVertexInputBindingDescription getBindingDescription() const;

  void setup();
  void update();
  void cleanup();

  VkWriteDescriptorSet getDescriptor(DescriptorHolder &holder, int i);

  template <typename T>
  AttributeDescriptor &
  attach(const std::function<void(const Mesh &m, T *)> &updateFn) {
    size = sizeof(T);
    updateFunction = [updateFn](const Mesh &m, void *d) {
      updateFn(m, (T *)d);
    };
    return *this;
  }

  AttributeDescriptor &onlySetup(bool val = true);
  AttributeDescriptor &computeFriendly(bool val = true);
  AttributeDescriptor &singleBuffered(bool val = true);

  template <typename T> AttributeDescriptor &attach() {
    return attach<T>([](const Mesh &, T *) {});
  }

  std::shared_ptr<AttributeDescriptor> clone(bool keepBuffers = false) const {
    const AttributeDescriptor tmp(*this);
    std::shared_ptr<AttributeDescriptor> cloned =
        std::make_shared<AttributeDescriptor>(tmp);
    if (keepBuffers)
      cloned->bufferId = bufferId;
    return cloned;
  }
  AttributeDescriptor(const AttributeDescriptor &from) = default;
  AttributeDescriptor(AttributeDescriptor &from) = delete;

  void previousFrame(bool val) { usesPreviousFrame = val; };
  int getBinding() const { return binding; }

protected:
  bool usesPreviousFrame;
  int binding;

  friend class ::Renderer;
  friend class Computer;
  friend class ComputeParams;
  friend class RenderParams;
  friend class BaseParams;
  DescriptorHolder *holder;
  AttributeRate rate;
  std::vector<std::pair<VkDeviceSize, VkFormat>> offsets;
  int size;

  bool isOnlySetup;
  bool isSingleBuffered;
  bool isComputeFriendly;
  VkDescriptorBufferInfo storageBufferInfo;

  std::function<void(const Mesh &m, void *)> updateFunction;
};

} // namespace Flim
