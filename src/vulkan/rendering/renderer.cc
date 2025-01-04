#include "renderer.hh"

#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/tree/camera.hh"
#include "api/tree/instance.hh"
#include "vulkan/buffers/buffer_utils.hh"
#include <Eigen/src/Core/Matrix.h>
#include <cstddef>
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
    desc.second->update(*this, mesh, cam);
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

void Renderer::createDescriptorSetLayout() {
  std::vector<VkDescriptorSetLayoutBinding> bindings(
      params.getUniformDescriptors().size());
  int i = 0;
  for (auto desc : params.getUniformDescriptors()) {
    bindings[i] = {};
    bindings[i].binding = desc.second->binding;
    bindings[i].descriptorType = desc.second->type;
    bindings[i].descriptorCount = 1;
    bindings[i].stageFlags = desc.second->stage;
    i++;
  }
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();
  if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

void Renderer::createDescriptorSets() {
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                             descriptorSetLayout);
  // Allocates descriptors from the pool
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  allocInfo.pSetLayouts = layouts.data();

  descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(context.device, &allocInfo,
                               descriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets!");
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

    std::vector<VkWriteDescriptorSet> descriptorWrites(
        params.getUniformDescriptors().size());

    int cur = 0;
    for (auto desc : params.getUniformDescriptors()) {
      descriptorWrites[cur++] = desc.second->getDescriptor(*this, i);
    }
    vkUpdateDescriptorSets(context.device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void Renderer::createDescriptorPool() {

  std::vector<VkDescriptorPoolSize> poolSizes(
      params.getUniformDescriptors().size());
  int i = 0;
  for (auto desc : params.getUniformDescriptors()) {
    poolSizes[i].type = desc.second->type;
    poolSizes[i].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    i++;
  }
  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  if (vkCreateDescriptorPool(context.device, &poolInfo, nullptr,
                             &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}
