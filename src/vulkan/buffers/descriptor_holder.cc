#include "descriptor_holder.hh"
#include "api/parameters/base_params.hh"

void DescriptorHolder::cleanupDescriptors()
{
  for (auto desc : params.getUniformDescriptors()) {
    desc.second->cleanup(*this);
  }

  for (auto desc : params.getAttributeDescriptors()) {
    desc.second->cleanup(*this);
  }
  vkDestroyDescriptorPool(context.device, descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(context.device, descriptorSetLayout, nullptr);
}

void DescriptorHolder::setupDescriptors() {
  for (auto &desc : params.getAttributeDescriptors()) {
    desc.second->setup(*this);
  }
  for (auto desc : params.getUniformDescriptors()) {
    desc.second->setup(*this);
  }
  createDescriptorSetLayout();
  createDescriptorPool();
  createDescriptorSets();
}


void DescriptorHolder::createDescriptorSetLayout() {
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

void DescriptorHolder::createDescriptorSets() {
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

void DescriptorHolder::createDescriptorPool() {

  std::vector<VkDescriptorPoolSize> poolSizes(
      params.getUniformDescriptors().size());
  int i = 0;
  for (auto desc : params.getUniformDescriptors()) {
    poolSizes[i].type = desc.second->getType();
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
