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
#include <iostream>
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

class Renderer : public DescriptorHolder {
public:
  Renderer(Renderer &) = delete;
  Renderer() = delete;
  void update();
  void setup();
  void cleanup();

  void setupUniforms();
  void updateUniforms(const Flim::Instance &obj, const Flim::Camera &cam);

  const std::vector<Flim::Instance> &getInstances();
  Flim::Mesh &mesh;
  Flim::RenderParams &params;
  Buffer indexBuffer;
  Pipeline pipeline;

  Renderer(Flim::Mesh &mesh, Flim::RenderParams &params)
      : DescriptorHolder(params, false), params(params), version(0), mesh(mesh),
        indexBuffer(mesh.indices.data(),
                    mesh.indices.size() * sizeof(mesh.indices[0]),
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
        pipeline(*this) {
    for (auto &attr : this->params.getAttributeDescriptors()) {
      CHECK(
          attr.second->getAttachedMesh() == nullptr,
          "You cannot reuse a render param for a mesh, please clone it first");
      bufferManager.attachMesh(attr.second->bufferId, &mesh);
    }
  };

#define CUR_FRAME -1
  template <typename Type>
  Kokkos::View<Type *, Kokkos::DefaultExecutionSpace>
  getAttributeBufferView(int binding, ssize_t frame = CUR_FRAME) const {
    for (auto &attr : params.attributes) {
      if (attr.second->binding == binding)
        return attr.second->getBuffer(frame)->getView<Type>();
    }
    throw std::runtime_error("Invalid binding");
  };

  Kokkos::View<Vector3i *, Kokkos::DefaultExecutionSpace>
  getIndexBufferView() const {
    return indexBuffer.getView<Vector3i>();
  }

private:
  int version;
};
