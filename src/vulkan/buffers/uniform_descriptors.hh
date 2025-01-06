#pragma once
#include "api/render/mesh.hh"
#include "api/tree/camera.hh"
#include <functional>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace Flim {
class Mesh;
};

class Renderer;

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

  virtual VkWriteDescriptorSet getDescriptor(Renderer &renderer, int i) = 0;
  virtual void setup(Renderer &renderer) = 0;
  virtual void update(Renderer &renderer, const Flim::Camera &cam) = 0;
  virtual void cleanup(Renderer &) {};
  virtual ~UniformDescriptor() {};

  virtual std::shared_ptr<UniformDescriptor> clone() const = 0;

  int getBinding() const {
    return binding;
  }

protected:
  const int binding;
  int stage;
  const int id;
  VkDescriptorType type;
  friend class Renderer;
};

class ImageUniDesc : public UniformDescriptor {
public:
  ImageUniDesc(int binding, std::string path, int shaderStage);

  void setup(Renderer &renderer) override;
  void update(Renderer &, const Flim::Camera &) override {};
  void cleanup(Renderer &renderer) override;

  ImageUniDesc &setType(VkDescriptorType type);

  VkWriteDescriptorSet getDescriptor(Renderer &renderer, int i) override;

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

  VkWriteDescriptorSet getDescriptor(Renderer &renderer, int i) override;

  void setup(Renderer &renderer) override;

  virtual void update(Renderer &renderer, const Flim::Camera &cam) override;
  void cleanup(Renderer &renderer) override;

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
