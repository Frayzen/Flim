#pragma once

#include <fwd.hh>
#include <vulkan/vulkan_core.h>

struct UniformLocationObject {
  Matrix4f model;
  Matrix4f view;
  Matrix4f proj;
};

struct UniformMaterialObject {
  alignas(16) Vector3f ambient;
  alignas(16) Vector3f diffuse;
  alignas(16) Vector3f specular;
};

class BufferManager {
public:
  BufferManager() = default;

  void createDepthResources();

private:
  std::vector<Buffer> uniformLocationBuffers;
  std::vector<void *> uniformLocationBuffersMapped;

  std::vector<Buffer> uniformMaterialBuffers;
  std::vector<void *> uniformMaterialBuffersMapped;
};
