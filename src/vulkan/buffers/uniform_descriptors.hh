#pragma once
#include "vulkan/buffers/buffer_manager.hh"
#include "vulkan/buffers/descriptor_holder.hh"
#include <functional>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace Flim {
class Mesh;
};

class DescriptorHolder;

#define VERTEX_SHADER_STAGE VK_SHADER_STAGE_VERTEX_BIT
#define FRAGMENT_SHADER_STAGE VK_SHADER_STAGE_FRAGMENT_BIT
#define COMPUTE_SHADER_STAGE VK_SHADER_STAGE_COMPUTE_BIT
#define ALL_SHADER_STAGE                                                       \
  VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT |                  \
      VK_SHADER_STAGE_COMPUTE_BIT

class UniformDescriptor : public BufferHolder {
public:
  UniformDescriptor() = delete;
  UniformDescriptor(int binding, int shaderStage, VkDescriptorType type)
      : binding(binding), type(type), stage(shaderStage) {};

  virtual VkWriteDescriptorSet getDescriptor(DescriptorHolder &holder,
                                             int i) = 0;
  virtual void setup() = 0;
  virtual void update() = 0;
  virtual void cleanup() {};
  virtual ~UniformDescriptor() {};

  virtual std::shared_ptr<UniformDescriptor> clone() const = 0;

  int getBinding() const { return binding; }
  VkDescriptorType getType() const { return type; }
  int getStage() const { return stage; }

protected:
  const int binding;
  int stage;
  VkDescriptorType type;
};

class ImageUniDesc : public UniformDescriptor {
public:
  ImageUniDesc(int binding, std::string path, int shaderStage);

  void setup() override;
  void update() override {};
  void cleanup() override;

  ImageUniDesc &setType(VkDescriptorType type);

  VkWriteDescriptorSet getDescriptor(DescriptorHolder &holder, int i) override;

  virtual std::shared_ptr<UniformDescriptor> clone() const override {
    return std::make_shared<ImageUniDesc>(*this);
  }

private:
  bool imageSetup;
  Image image;
  std::string path;
};

class GeneralUniDesc : public UniformDescriptor {

public:
  GeneralUniDesc(int binding, int shaderStage)
      : UniformDescriptor(binding, shaderStage,
                          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
        bufferSize(0) {};

  template <typename T>
  GeneralUniDesc &attach(const std::function<void(T *)> updateFn) {
    static_assert(
        std::is_scalar<T>() || alignof(T) % 2 == 0,
        "Non-scalar descriptor attchement should be aligned on 2 bytes. Please "
        "consider adding 'alignas(16)' after the struct keyword. Also check "
        "that every values in the struct are all individually also aligned on "
        "2 bytes. Please consider adding 'alignas(16)' before each attributes");
    bufferSize = sizeof(T);
    updateFunction = [updateFn](void *ptr) { updateFn(static_cast<T *>(ptr)); };
    return *this;
  };

  template <typename T> GeneralUniDesc &attachObj(const T &ref) {
    return attach<T>([&ref](T *ptr) {
      memcpy(static_cast<T *>(ptr), &ref, sizeof(T));
    });
  };

  VkWriteDescriptorSet getDescriptor(DescriptorHolder &holder, int i) override;

  void setup() override;
  virtual void update() override;
  void cleanup() override;

  virtual std::shared_ptr<UniformDescriptor> clone() const override {
    return std::make_shared<GeneralUniDesc>(*this);
  }

protected:
  size_t bufferSize;

  std::function<void(void *)> updateFunction;

  friend class DescriptorsManager;

private:
  VkDescriptorBufferInfo bufferInfo;
};
