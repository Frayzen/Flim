#include "descriptor_holder.hh"
#include "api/parameters/base_params.hh"
#include "vulkan/context.hh"
#include <iostream>

void DescriptorHolder::cleanupDescriptors() {
  for (auto &desc : params.getUniformDescriptors()) {
    desc.second->cleanup();
  }

  for (auto desc : params.getAttributeDescriptors()) {
    desc.second->cleanup();
  }
  vkDestroyDescriptorPool(context.device, descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(context.device, descriptorSetLayout, nullptr);
}

void DescriptorHolder::setupDescriptors() {
  for (auto &desc : params.getAttributeDescriptors()) {
    desc.second->setup();
  }
  for (auto desc : params.getUniformDescriptors()) {
    desc.second->setup();
  }
  createDescriptorSetLayout();
  createDescriptorPool();
  createDescriptorSets();
}

void DescriptorHolder::createDescriptorSetLayout() {
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  VkDescriptorSetLayoutBinding cur;
  for (auto desc : params.getUniformDescriptors()) {
    cur = {};
    cur.binding = desc.second->getBinding();
    cur.descriptorType = desc.second->getType();
    cur.descriptorCount = 1;
    cur.stageFlags = desc.second->getStage();
    bindings.push_back(cur);
  }
  if (isComputeHolder)
    for (auto desc : params.getAttributeDescriptors()) {
      cur = {};
      cur.binding = desc.second->getBinding();
      cur.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      cur.descriptorCount = 1;
      cur.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
      bindings.push_back(cur);
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

int DescriptorHolder::getDescriptorsSize() const {
  int descsSize = params.getUniformDescriptors().size();
  if (isComputeHolder)
    descsSize += params.getAttributeDescriptors().size();
  return descsSize;
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

  std::vector<VkWriteDescriptorSet> descriptorWrites(getDescriptorsSize());
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

    int cur = 0;
    for (auto desc : params.getUniformDescriptors()) {
      descriptorWrites[cur++] = desc.second->getDescriptor(*this, i);
    }
    if (isComputeHolder) {
      for (auto &desc : params.getAttributeDescriptors()) {
        descriptorWrites[cur++] = desc.second->getDescriptor(*this, i);
      }
    }
    vkUpdateDescriptorSets(context.device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void DescriptorHolder::createDescriptorPool() {

  std::vector<VkDescriptorPoolSize> poolSizes(getDescriptorsSize());
  int i = 0;
  for (auto desc : params.getUniformDescriptors()) {
    poolSizes[i].type = desc.second->getType();
    poolSizes[i].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    i++;
  }
  if (isComputeHolder) {
    for (auto desc : params.getAttributeDescriptors()) {
      poolSizes[i].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      poolSizes[i].descriptorCount =
          static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2;
      i++;
    }
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

void DescriptorHolder::printBufferIds() const {
  for (auto &a : params.getAttributeDescriptors())
    std::cout << a.second->getBufferId() << '\n';
}
