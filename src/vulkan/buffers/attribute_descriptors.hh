#pragma once

#include "vulkan/buffers/buffer_manager.hh"
#include <Eigen/Core>
#include <functional>
#include <fwd.hh>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

class DescriptorHolder;
namespace Flim {

class Mesh;

enum AttributeRate {
  VERTEX,
  INSTANCE,
};

class BaseAttributeDescriptor : public BufferHolder {
public:
  BaseAttributeDescriptor(Mesh &m, int binding)
      : BufferHolder(), mesh(&m), binding(binding), usesPreviousFrame(false) {};
  virtual int getAmountOffset() const = 0;
  virtual VkVertexInputAttributeDescription getAttributeDesc(int id) const = 0;
  virtual VkVertexInputBindingDescription getBindingDescription() const = 0;

  virtual void setup() = 0;
  virtual void update() = 0;
  virtual void cleanup() = 0;

  virtual VkWriteDescriptorSet getDescriptor(DescriptorHolder &holder,
                                             int i) = 0;

  virtual std::shared_ptr<BaseAttributeDescriptor> clone() const = 0;

  void previousFrame(bool val) { usesPreviousFrame = val; };
  int getBinding() const { return binding; }

protected:
  Mesh *mesh;
  bool usesPreviousFrame;
  int binding;
  friend class Computer;
  friend class RenderParams;
  friend class BaseParams;
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

class AttributeDescriptor : public BaseAttributeDescriptor {

public:
  AttributeDescriptor(Mesh &mesh, int binding, AttributeRate rate);

  AttributeDescriptor &add(long offset, VkFormat format);

  int getAmountOffset() const override;
  VkVertexInputAttributeDescription getAttributeDesc(int id) const override;
  VkVertexInputBindingDescription getBindingDescription() const override;

  void setup() override;
  void update() override;
  void cleanup() override;

  VkWriteDescriptorSet getDescriptor(DescriptorHolder &holder, int i) override;

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

  virtual std::shared_ptr<BaseAttributeDescriptor> clone() const override {
    return std::make_shared<AttributeDescriptor>(*this);
  }

private:
  DescriptorHolder *holder;
  AttributeRate rate;
  std::vector<std::pair<long, VkFormat>> offsets;
  int size;

  bool isOnlySetup;
  bool isSingleBuffered;
  bool isComputeFriendly;
  VkDescriptorBufferInfo storageBufferInfo;

  std::function<void(const Mesh &m, void *)> updateFunction;
};

} // namespace Flim
