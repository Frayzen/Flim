#pragma once

#include <fwd.hh>
#include <vulkan/vulkan_core.h>

struct UniformLocationObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

struct UniformMaterialObject {
  alignas(16) glm::vec3 ambient;
  alignas(16) glm::vec3 diffuse;
  alignas(16) glm::vec3 specular;
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
