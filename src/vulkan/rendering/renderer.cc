#include "renderer.hh"

#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/tree/instance.hh"
#include "vulkan/rendering/rendering_context.hh"
#include <Eigen/src/Core/Matrix.h>
#include <vector>
#include <vulkan/vulkan_core.h>

void Renderer::setup() {
  context.rctx.mesh = &mesh;
  assert(mesh.vertices.size() > 0);
  assert(mesh.indices.size() > 0);
  setupDescriptors();
  pipeline.create();
}

const std::vector<Flim::Instance> &Renderer::getInstances() {
  return mesh.instances;
}

void Renderer::update() {
  context.rctx.mesh = &mesh;
  for (auto desc : params.getUniformDescriptors()) {
    desc.second->update();
  }
  for (auto desc : params.getAttributeDescriptors()) {
    desc.second->update();
  }
  if (params.version != version) {
    vkDeviceWaitIdle(context.device);
    pipeline.cleanup();
    pipeline.create();
    version = params.version;
  }
}

void Renderer::cleanup() {
  pipeline.cleanup();
  cleanupDescriptors();
  indexBuffer.destroy();
}
