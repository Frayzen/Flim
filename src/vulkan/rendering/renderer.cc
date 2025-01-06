#include "renderer.hh"

#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/tree/camera.hh"
#include "api/tree/instance.hh"
#include "vulkan/buffers/buffer_utils.hh"
#include <Eigen/src/Core/Matrix.h>
#include <vector>
#include <vulkan/vulkan_core.h>

void Renderer::setup() {
  assert(mesh.vertices.size() > 0);
  assert(mesh.indices.size() > 0);

  createBufferFromData(indexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                       mesh.indices.data(),
                       mesh.indices.size() * sizeof(mesh.indices[0]));

  for (auto &desc : params.getAttributeDescriptors()) {
    desc.second->setup(*this);
  }
  for (auto desc : params.getUniformDescriptors()) {
    desc.second->setup(*this);
  }
  createDescriptorSetLayout();
  createDescriptorPool();
  createDescriptorSets();
  pipeline.create();
}

const std::vector<Flim::Instance> &Renderer::getInstances() {
  return mesh.instances;
}

void Renderer::update(const Flim::Camera &cam) {
  for (auto desc : params.getUniformDescriptors()) {
    desc.second->update(*this, cam);
  }
  for (auto desc : params.getAttributeDescriptors()) {
    desc.second->update(*this);
  }
  if (params.version != version) {
    vkDeviceWaitIdle(context.device);
    pipeline.cleanup();
    pipeline.create();
    version = params.version;
  }
}

void Renderer::cleanup() {
  destroyBuffer(indexBuffer);
  for (auto desc : params.getUniformDescriptors()) {
    desc.second->cleanup(*this);
  }

  for (auto desc : params.getAttributeDescriptors()) {
    desc.second->cleanup(*this);
  }
  pipeline.cleanup();
  vkDestroyDescriptorPool(context.device, descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(context.device, descriptorSetLayout, nullptr);
}
