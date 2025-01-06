#pragma once

#include "consts.hh"
#include "vulkan/rendering/renderer.hh"
#include <Eigen/Core>
#include <functional>
#include <fwd.hh>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace Flim {

enum AttributeRate {
  VERTEX,
  INSTANCE,
};

static int attrid = 0;

class BaseAttributeDescriptor {
public:
  BaseAttributeDescriptor(int binding)
      : id(attrid++), binding(binding), usesPreviousFrame(false) {};
  virtual int getAmountOffset() const = 0;
  virtual VkVertexInputAttributeDescription getAttributeDesc(int id) const = 0;
  virtual VkVertexInputBindingDescription getBindingDescription() const = 0;

  virtual void update(Renderer &renderer) = 0;
  virtual void setup(Renderer &renderer) = 0;
  virtual void cleanup(Renderer &renderer) = 0;

  virtual const Buffer &getBuffer(const Renderer &renderer,
                                  int currentImage) const = 0;

  virtual std::shared_ptr<BaseAttributeDescriptor> clone() const = 0;
  const int id;

  void previousFrame(bool val) { usesPreviousFrame = val; };
  int getBinding() const { return binding; }

protected:
  bool usesPreviousFrame;
  int binding;
  friend class Computer;
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
  AttributeDescriptor(int binding, AttributeRate rate);

  AttributeDescriptor &add(long offset, VkFormat format);

  int getAmountOffset() const override;
  VkVertexInputAttributeDescription getAttributeDesc(int id) const override;
  VkVertexInputBindingDescription getBindingDescription() const override;

  void update(Renderer &renderer) override;
  void setup(Renderer &renderer) override;
  void cleanup(Renderer &renderer) override;

  template <typename T>
  AttributeDescriptor &
  attach(const std::function<void(const Mesh &, T *)> &updateFn) {
    size = sizeof(T);
    updateFunction = [updateFn](const Mesh &m, void *d) {
      updateFn(m, (T *)d);
    };
    return *this;
  }

  AttributeDescriptor &onlySetup(bool val = true);
  AttributeDescriptor &computeFriendly(bool val = true);

  const Buffer &getBuffer(const Renderer &renderer,
                          int currentImage) const override;

  template <typename T> AttributeDescriptor &attach() {
    return attach<T>([](const Mesh &, T *) {});
  }

  virtual std::shared_ptr<BaseAttributeDescriptor> clone() const override {
    return std::make_shared<AttributeDescriptor>(*this);
  }

private:
  int redudancyAmount() const {
    if (isOnlySetup && !isComputeFriendly)
      return 1;
    return MAX_FRAMES_IN_FLIGHT;
  }

  AttributeRate rate;
  std::vector<std::pair<long, VkFormat>> offsets;
  int size;

  bool isOnlySetup;
  bool isComputeFriendly;

  std::function<void(const Mesh &, void *)> updateFunction;
};

} // namespace Flim
