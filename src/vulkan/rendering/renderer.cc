#include "api/parameters.hh"
#include "api/tree/camera_object.hh"

void Renderer::setup() {
  for (auto desc : params.descriptors) {
    desc->setup(*this);
  }
  createDescriptorSetLayout();
  createDescriptorPool();
  createDescriptorSets();
  pipeline.create();
}

void Renderer::update(const Flim::CameraObject &cam) {
  for (auto desc : params.descriptors) {
    desc->update(*this, mesh, cam);
  }
  if (params.version != version) {
    vkDeviceWaitIdle(context.device);
    pipeline.cleanup();
    pipeline.create();
    version = params.version;
  }
}

void Renderer::cleanup() {
  mesh.cleanup();
  for (auto desc : params.descriptors) {
    desc->cleanup(*this);
  }
  pipeline.cleanup();
  vkDestroyDescriptorPool(context.device, descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(context.device, descriptorSetLayout, nullptr);
}

void Renderer::createDescriptorSetLayout() {
  std::vector<VkDescriptorSetLayoutBinding> bindings(params.descriptors.size());
  int i = 0;
  for (auto desc : params.descriptors) {
    bindings[i] = {};
    bindings[i].binding = desc->binding;
    bindings[i].descriptorType = desc->type;
    bindings[i].descriptorCount = 1;
    bindings[i].stageFlags = shaderStageToVulkanFlags(desc->stage);
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
        params.descriptors.size());

    int cur = 0;
    for (auto desc : params.descriptors) {
      descriptorWrites[cur++] = desc->getDescriptor(*this, i);
    }
    vkUpdateDescriptorSets(context.device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void Renderer::createDescriptorPool() {

  std::vector<VkDescriptorPoolSize> poolSizes(params.descriptors.size());
  int i = 0;
  for (auto desc : params.descriptors) {
    poolSizes[i].type = desc->type;
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