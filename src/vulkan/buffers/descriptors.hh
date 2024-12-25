#pragma once

#include "api/tree/free_camera_object.hh"
#include "api/tree/instance_object.hh"
#include "vulkan/context.hh"
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
  void update(const Flim::InstanceObject &object,
              const Flim::CameraObject &cam) override {
    (void)object;
    (void)cam;
  };

private:
  Image image;
  std::string path;
};

class GeneralDescriptor : public BaseDescriptor {

public:
  GeneralDescriptor(int binding)
      : BaseDescriptor(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {};

  template <typename T>
  GeneralDescriptor &attach(std::function<void(const Flim::InstanceObject &,
                                               const Flim::CameraObject &, T &)>
                                updateFn) {
    bufferSize = sizeof(T);
    updateFunction = [&](const Flim::InstanceObject &obj,
                         const Flim::CameraObject &cam) {
      updateFn(obj, cam, *((T *)mappedBuffers[context.currentImage]));
    };
    return *this;
  };

  VkWriteDescriptorSet getDescriptor(int i) override;

  void setup() override;

  virtual void update(const Flim::InstanceObject &object,
                      const Flim::CameraObject &cam) override {
    updateFunction(object, cam);
  };

  void cleanup() override {
    for (size_t i = 0; i < buffers.size(); i++) {
      vkDestroyBuffer(context.device, buffers[i].buffer, nullptr);
      vkFreeMemory(context.device, buffers[i].bufferMemory, nullptr);
    }
  };

protected:
  size_t bufferSize;

  std::vector<void *> mappedBuffers;
  std::vector<Buffer> buffers;

  std::function<void(const Flim::InstanceObject &, const Flim::CameraObject &)>
      updateFunction;

  friend class DescriptorsManager;
};
