#pragma once

#include "api/tree/camera_object.hh"
#include "api/tree/instance_object.hh"
#include "vulkan/context.hh"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
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

class BaseDescriptor {
public:
  BaseDescriptor(int binding, VkDescriptorType type)
      : binding(binding), type(type), stage(ShaderStage::Both) {};
  // /!\ omit the dstSet value here
  virtual VkWriteDescriptorSet getDescriptor(int i) = 0;
  virtual void setup() = 0;
  virtual void update(const Flim::InstanceObject &object,
                      const Flim::CameraObject &cam) = 0;
  virtual void cleanup() {};
  virtual ~BaseDescriptor() {};

protected:
  ShaderStage stage;
  int binding;
  VkDescriptorType type;
  friend class DescriptorsManager;
};

class ImageDescriptor : public BaseDescriptor {
public:
  ImageDescriptor(int binding, std::string path);
  VkWriteDescriptorSet getDescriptor(int i) override;

  void setup() override;
  void update(const Flim::InstanceObject &,
              const Flim::CameraObject &) override {};

  void cleanup() override;

private:
  Image image;
  std::string path;
};

// Concept to check for a specific method signature
template <typename T>
concept HasUpdateMethod = requires(const Flim::InstanceObject &instance,
                                   const Flim::CameraObject &cam, T *ptr) {
  { T::update(instance, cam, ptr) } -> std::same_as<void>;
};

class GeneralDescriptor : public BaseDescriptor {

public:
  GeneralDescriptor(int binding)
      : BaseDescriptor(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
        bufferSize(0) {};

  template <HasUpdateMethod T> GeneralDescriptor &attach() {
    bufferSize = sizeof(T);
    // Cast to T::update<void(T*)>
    updateFunction = [](const Flim::InstanceObject &istc,
                        const Flim::CameraObject &cam, void *ptr) {
      T::update(istc, cam, static_cast<T *>(ptr));
    };
    return *this;
  };

  VkWriteDescriptorSet getDescriptor(int i) override;

  void setup() override;

  virtual void update(const Flim::InstanceObject &object,
                      const Flim::CameraObject &cam) override {
    void *curBuf = mappedBuffers[context.currentImage];
    updateFunction(object, cam, curBuf);
  };

  void cleanup() override;

protected:
  size_t bufferSize;

  std::vector<void *> mappedBuffers;
  std::vector<Buffer> buffers;

  std::function<void(const Flim::InstanceObject &, const Flim::CameraObject &,
                     void *)>
      updateFunction;

  friend class DescriptorsManager;

private:
  VkDescriptorBufferInfo bufferInfo;
};
