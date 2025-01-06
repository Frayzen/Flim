#pragma once
#include "api/render/mesh.hh"
#include "api/tree/camera.hh"
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

static int uniid = 0;

class UniformDescriptor {
public:
  UniformDescriptor() = delete;
  UniformDescriptor(int binding, int shaderStage, VkDescriptorType type)
      : binding(binding), type(type), stage(shaderStage), id(uniid++) {};

  virtual VkWriteDescriptorSet getDescriptor(DescriptorHolder &holder, int i) = 0;
  virtual void setup(DescriptorHolder &holder) = 0;
  virtual void update(DescriptorHolder &holder, const Flim::Camera &cam) = 0;
  virtual void cleanup(DescriptorHolder &) {};
  virtual ~UniformDescriptor() {};

  virtual std::shared_ptr<UniformDescriptor> clone() const = 0;

  int getBinding() const {
    return binding;
  }
  VkDescriptorType getType() const {
    return type;
  }
  int getStage() const {
    return stage;
  }

protected:
  const int binding;
  int stage;
  const int id;
  VkDescriptorType type;
  friend class DescriptorHolder;
};

class ImageUniDesc : public UniformDescriptor {
public:
  ImageUniDesc(int binding, std::string path, int shaderStage);

  void setup(DescriptorHolder &holder) override;
  void update(DescriptorHolder &, const Flim::Camera &) override {};
  void cleanup(DescriptorHolder &holder) override;

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
  GeneralUniDesc &attach(
      const std::function<void(const Flim::Mesh &, const Flim::Camera &, T *)>
          updateFn) {
    static_assert(
        std::is_scalar<T>() || alignof(T) % 2 == 0,
        "Non-scalar descriptor attchement should be aligned on 2 bytes. Please "
        "consider adding 'alignas(16)' after the struct keyword. Also check "
        "that every values in the struct are all individually also aligned on "
        "2 bytes. Please consider adding 'alignas(16)' before each attributes");
    bufferSize = sizeof(T);
    updateFunction = [updateFn](const Flim::Mesh &mesh, const Flim::Camera &cam,
                                void *ptr) {
      updateFn(mesh, cam, static_cast<T *>(ptr));
    };
    return *this;
  };

  template <typename T> GeneralUniDesc &attachObj(const T &ref) {
    return attach<T>([&ref](const Flim::Mesh &, const Flim::Camera &, T *ptr) {
      memcpy(static_cast<T *>(ptr), &ref, sizeof(T));
    });
  };

  VkWriteDescriptorSet getDescriptor(DescriptorHolder &holder, int i) override;

  void setup(DescriptorHolder &holder) override;

  virtual void update(DescriptorHolder &holder, const Flim::Camera &cam) override;
  void cleanup(DescriptorHolder &holder) override;

  virtual std::shared_ptr<UniformDescriptor> clone() const override {
    return std::make_shared<GeneralUniDesc>(*this);
  }

protected:
  size_t bufferSize;

  std::function<void(const Flim::Mesh &, const Flim::Camera &, void *)>
      updateFunction;

  friend class DescriptorsManager;

private:
  VkDescriptorBufferInfo bufferInfo;
};
