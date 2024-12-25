#include "descriptors_manager.hh"
#include "vulkan/context.hh"
#include <cassert>

void DescriptorsManager::updateDescriptors() {
  descriptors = &context.renderer->descriptors;
}

void DescriptorsManager::createDescriptorSetLayout() {
  assert(descriptors != nullptr);
  std::vector<VkDescriptorSetLayoutBinding> bindings(descriptors->size());
  int i = 0;
  for (auto desc : *descriptors) {
    VkDescriptorSetLayoutBinding locationLayoutBinding{};
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
                                  &context.descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

void DescriptorsManager::createDescriptorPool() {
  assert(descriptors != nullptr);
  std::vector<VkDescriptorPoolSize> poolSizes(descriptors->size());
  int i = 0;
  for (auto desc : *descriptors) {
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
                             &context.descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

void DescriptorsManager::createUniformBuffers() {
  assert(descriptors != nullptr);
  for (auto desc : *descriptors) {
    if (auto genDesc = std::dynamic_pointer_cast<GeneralDescriptor>(desc)) {
      VkDeviceSize bufferSize = genDesc->bufferSize;
      genDesc->setup();
    }
  }
}

void DescriptorsManager::createDescriptorSets() {
  assert(descriptors != nullptr);
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                             context.descriptorSetLayout);
  // Allocates descriptors from the pool
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = context.descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  allocInfo.pSetLayouts = layouts.data();

  context.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(context.device, &allocInfo,
                               context.descriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets!");
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

    std::vector<VkWriteDescriptorSet> descriptorWrites(descriptors->size());

    int cur = 0;
    for (auto desc : *descriptors) {
      // set dstSet
      descriptorWrites[cur++] = desc->getDescriptor(i);
      descriptorWrites[cur++].dstSet = context.descriptorSets[i];
    }
    vkUpdateDescriptorSets(context.device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void DescriptorsManager::updateUniformBuffer(const Flim::InstanceObject &object,
                                             const Flim::CameraObject *cam) {
  assert(descriptors != nullptr);
  for (auto desc : *descriptors) {
    desc->update(object, *cam);
  }
}
