#include "renderer.hh"

#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/tree/instance.hh"
#include "vulkan/context.hh"
#include <Eigen/src/Core/Matrix.h>
#include <iostream>
#include <vector>
#include <vulkan/vulkan_core.h>

void Renderer::setup() {
  assert(mesh.vertices.size() > 0);
  assert(mesh.indices.size() > 0);
  setupDescriptors();
  pipeline->create();
}

const std::vector<Flim::Instance> &Renderer::getInstances() {
  return mesh.instances;
}

void Renderer::update() {
  for (auto desc : params.getUniformDescriptors()) {
    desc.second->update();
  }
  for (auto desc : params.getAttributeDescriptors()) {
    desc.second->update();
  }
  if (params.version != version) {
    vkDeviceWaitIdle(context.device); // not ideal, might change later
    pipeline = std::make_unique<Pipeline>(*this);
    pipeline->create();
    std::cout << "RECREATED " << std::endl;
    version = params.version;
  }
  VkDrawIndexedIndirectCommand cmd{
      .indexCount = static_cast<uint32_t>(mesh.indices.size()),
      .instanceCount = static_cast<uint32_t>(mesh.instances.size()),
      .firstIndex = 0,
      .vertexOffset = 0,
      .firstInstance = 0,
  };
  drawCmdBuffer = std::make_unique<Buffer>(
      "Draw command buffer mesh id " + std::to_string(mesh.id), &cmd,
      sizeof(cmd), VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT);
}

const Buffer &Renderer::getDrawCommandBuffer() const { return *drawCmdBuffer; }
