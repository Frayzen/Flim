#pragma once

#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/tree/camera.hh"
#include "utils/checks.hh"
#include "vulkan/buffers/buffer_manager.hh"
#include "vulkan/buffers/descriptor_holder.hh"
#include "vulkan/context.hh"
#include "vulkan/rendering/pipeline.hh"
#include <Eigen/src/Core/Matrix.h>
#include <stdexcept>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>
namespace Flim {
class Scene;
class RenderParams;
class Instance;
class Mesh;
}; // namespace Flim

// An abstract renderable object (a mesh)
class Renderer : public DescriptorHolder {
public:
  Renderer(Renderer &) = delete;
  Renderer() = delete;
  void update();
  void setup();

  const Buffer &getDrawCommandBuffer() const;

  void setupUniforms();
  void updateUniforms(const Flim::Instance &obj, const Flim::Camera &cam);

  const std::vector<Flim::Instance> &getInstances();
  Flim::Mesh &mesh;
  Flim::RenderParams &params;
  Buffer indexBuffer;
  std::unique_ptr<Pipeline> pipeline;

  Renderer(Flim::Mesh &mesh, Flim::RenderParams &params)
      : DescriptorHolder(params, false), params(params), version(0), mesh(mesh),
        indexBuffer("Index buffer", mesh.indices.data(),
                    mesh.indices.size() * sizeof(mesh.indices[0]),
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0, true),
        pipeline(std::make_unique<Pipeline>(*this)) {
    for (auto &attr : this->params.getAttributeDescriptors()) {
      CHECK(
          attr.second->getAttachedMesh() == nullptr,
          "You cannot reuse a render param for a mesh, please clone it first");
      bufferManager.attachMesh(attr.second->bufferId, &mesh);
    }
  };

private:
  int version;
  std::unique_ptr<Buffer> drawCmdBuffer;
};
