#pragma once

#include "api/tree/camera_object.hh"
#include "vulkan/context.hh"
#include "vulkan/rendering/renderer.hh"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <string>
#include <vulkan/vulkan_core.h>

enum ShaderStage {
  Vertex = 0,
  Fragment,
  Both,
};

inline VkShaderStageFlags shaderStageToVulkanFlags(ShaderStage stage) {
  switch (stage) {
  case ShaderStage::Vertex:
    return VK_SHADER_STAGE_VERTEX_BIT;
  case ShaderStage::Fragment:
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  case ShaderStage::Both:
    return VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  default:
    std::runtime_error("Unexepected shared stage");
    return 0;
  }
}

static int curid = 0;
class BaseDescriptor {
public:
  BaseDescriptor() = delete;
  BaseDescriptor(int binding, VkDescriptorType type)
      : binding(binding), type(type), stage(ShaderStage::Both), id(curid++) {};
  virtual VkWriteDescriptorSet getDescriptor(Renderer &renderer, int i) = 0;
  virtual void setup(Renderer &renderer) = 0;
  virtual void update(Renderer &renderer, const Flim::Mesh &mesh,
                      const Flim::CameraObject &cam) = 0;
  virtual void cleanup(Renderer &) {};
  virtual ~BaseDescriptor() {};

protected:
  ShaderStage stage;
  int id;
  int binding;
  VkDescriptorType type;
  friend class Renderer;
};

class ImageDescriptor : public BaseDescriptor {
public:
  ImageDescriptor(int binding, std::string path);
  VkWriteDescriptorSet getDescriptor(Renderer &renderer, int i) override;

  void setup(Renderer &renderer) override;
  void update(Renderer &, const Flim::Mesh &,
              const Flim::CameraObject &) override {};

  void cleanup(Renderer &renderer) override;

private:
  bool imageSetup;
  Image image;
  std::string path;
};

// Concept to check for a specific method signature
template <typename T>
concept HasUpdateMethod = requires(const Flim::Mesh &mesh,
                                   const Flim::CameraObject &cam, T *ptr) {
  { T::update(mesh, cam, ptr) } -> std::same_as<void>;
};

class GeneralDescriptor : public BaseDescriptor {

public:
  GeneralDescriptor(int binding)
      : BaseDescriptor(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
        bufferSize(0) {};

  template <HasUpdateMethod T> GeneralDescriptor &attach() {
    static_assert(
        std::is_scalar<T>() || alignof(T) % 2 == 0,
        "Non-scalar descriptor attchement should be aligned on 2 bytes. Please "
        "consider adding 'alignas(16)' after the struct keyword. Also check "
        "that every values in the struct are all individually also aligned on "
        "2 bytes. Please consider adding 'alignas(16)' before each attributes");
    bufferSize = sizeof(T);
    // Cast to T::update<void(T*)>
    updateFunction = [](const Flim::Mesh &mesh,
                        const Flim::CameraObject &cam, void *ptr) {
      T::update(mesh, cam, static_cast<T *>(ptr));
    };
    return *this;
  };

  template <typename T> GeneralDescriptor &attach(T &ref) {
    static_assert(
        std::is_scalar<T>() || alignof(T) % 2 == 0,
        "Non-scalar descriptor attchement should be aligned on 2 bytes. Please "
        "consider adding 'alignas(16)' after the struct keyword. Also check "
        "that every values in the struct are all individually also aligned on "
        "2 bytes. Please consider adding 'alignas(16)' before each attributes");
    bufferSize = sizeof(T);
    // Cast to T::update<void(T*)>
    updateFunction = [&](const Flim::Mesh &,
                         const Flim::CameraObject &, void *ptr) {
      memcpy(static_cast<T *>(ptr), &ref, bufferSize);
    };
    return *this;
  };

  VkWriteDescriptorSet getDescriptor(Renderer &renderer, int i) override;

  void setup(Renderer &renderer) override;

  virtual void update(Renderer &renderer, const Flim::Mesh &mesh,
                      const Flim::CameraObject &cam) override {
    void *curBuf = renderer.mappedUniforms[id][context.currentImage];
    updateFunction(mesh, cam, curBuf);
  };

  void cleanup(Renderer &renderer) override;

protected:
  size_t bufferSize;

  std::function<void(const Flim::Mesh &, const Flim::CameraObject &, void *)>
      updateFunction;

  friend class DescriptorsManager;

private:
  VkDescriptorBufferInfo bufferInfo;
};
