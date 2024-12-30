#pragma once

#include "api/tree/camera.hh"
#include "fwd.hh"
#include "vulkan/rendering/pipeline.hh"
#include <map>
#include <vector>
namespace Flim {
class RenderParams;
class Instance;
class Mesh;
}; // namespace Flim

class Renderer {
public:
  Renderer(Renderer &) = delete;
  Renderer() = delete;
  void setup();
  void cleanup();
  void update(const Flim::Camera &cam);

  void createDescriptorSetLayout();
  void setupUniforms();
  void updateUniforms(const Flim::Instance &obj,
                      const Flim::Camera &cam);
  void createDescriptorSets();
  void createDescriptorPool();

  Renderer(Flim::Mesh &mesh, Flim::RenderParams &params)
      : mesh(mesh), params(params), version(0), pipeline(*this),
        descriptorSetLayout(0), descriptorPool(0) {};
  Flim::Mesh &mesh;
  Flim::RenderParams &params;

  std::map<int, std::vector<Buffer>> uniforms;
  std::map<int, std::vector<void *>> mappedUniforms;

  Buffer indexBuffer;
  Buffer vertexBuffer;
  Buffer instancesMatrixBuffer;

  std::vector<VkDescriptorSet> descriptorSets;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;

private:
  Pipeline pipeline;
  int version;
  friend class CommandPoolManager;
};
