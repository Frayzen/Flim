#include "api/render/mesh.hh"
#include "api/tree/free_camera_object.hh"
#include "vulkan/buffers/buffer_utils.hh"
#include <chrono>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>
#include <vulkan/vulkan_core.h>

/* #define GLM_FORCE_LEFT_HANDED */
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // for perspective projection
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstring>
#include <stdexcept>

#include "buffer_manager.hh"

void BufferManager::createDescriptorPool() {
  std::array<VkDescriptorPoolSize, 3> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

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

void BufferManager::createDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding locationLayoutBinding{};
  // linked to the binding property of the vert shader
  locationLayoutBinding.binding = 0;
  locationLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  locationLayoutBinding.descriptorCount = 1;
  // This could be used to specify a transformation for each of the bones in a
  // skeleton for skeletal animation, for example
  locationLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  locationLayoutBinding.pImmutableSamplers = nullptr; // Optional

  VkDescriptorSetLayoutBinding materialLayoutBinding{};
  // linked to the binding property of the vert shader
  materialLayoutBinding.binding = 1;
  materialLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  materialLayoutBinding.descriptorCount = 1;
  materialLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 2;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 3> bindings = {
      locationLayoutBinding, materialLayoutBinding, samplerLayoutBinding};
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr,
                                  &context.descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

void BufferManager::createUniformBuffers() {
  VkDeviceSize locationBufferSize = sizeof(UniformLocationObject);
  VkDeviceSize materialBufferSize = sizeof(UniformMaterialObject);

  uniformLocationBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  uniformLocationBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

  uniformMaterialBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  uniformMaterialBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    createBuffer(locationBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 uniformLocationBuffers[i]);
    vkMapMemory(context.device, uniformLocationBuffers[i].bufferMemory, 0,
                locationBufferSize, 0, &uniformLocationBuffersMapped[i]);

    createBuffer(materialBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 uniformMaterialBuffers[i]);
    vkMapMemory(context.device, uniformMaterialBuffers[i].bufferMemory, 0,
                materialBufferSize, 0, &uniformMaterialBuffersMapped[i]);
  }
}

void BufferManager::createDescriptorSets() {
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

    VkDescriptorBufferInfo locationBufferInfo{};
    locationBufferInfo.buffer = uniformLocationBuffers[i].buffer;
    locationBufferInfo.offset = 0;
    locationBufferInfo.range = sizeof(UniformLocationObject);

    VkDescriptorBufferInfo materialBufferInfo{};
    materialBufferInfo.buffer = uniformMaterialBuffers[i].buffer;
    materialBufferInfo.offset = 0;
    materialBufferInfo.range = sizeof(UniformMaterialObject);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = context.images[0].view;
    imageInfo.sampler = context.images[0].sampler;

    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = context.descriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &locationBufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = context.descriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &materialBufferInfo;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = context.descriptorSets[i];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(context.device,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void BufferManager::updateUniformBuffer(const Flim::Mesh &mesh,
                                        const Flim::FreeCameraObject *cam) {
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(
                   currentTime - startTime)
                   .count();

  UniformLocationObject location{};
  location.model = mesh.transform.getViewMatrix();
  location.view = cam->transform.getViewMatrix();

  location.proj =
      glm::perspective(radians(cam->fov),
                       context.swapChain.swapChainExtent.width /
                           (float)context.swapChain.swapChainExtent.height,
                       cam->near, cam->far);
  location.proj[1][1] *= -1;
  memcpy(uniformLocationBuffersMapped[context.currentImage], &location,
         sizeof(location));

  UniformMaterialObject material{};
  material.ambient = mesh.getMaterial().ambient;
  material.diffuse = mesh.getMaterial().diffuse;
  material.specular = mesh.getMaterial().specular;

  /* std::cout << "=========== " << sizeof(material) << std::endl; */
  /* std::cout << glm::to_string(material.ambient) << std::endl; */
  /* std::cout << glm::to_string(mesh.getMaterial().ambient) << std::endl; */
  /* std::cout << glm::to_string(material.diffuse) << std::endl; */
  /* std::cout << glm::to_string(mesh.getMaterial().diffuse) << std::endl; */
  /* std::cout << glm::to_string(material.specular) << std::endl; */
  /* std::cout << glm::to_string(mesh.getMaterial().specular) << std::endl; */

  memcpy(uniformMaterialBuffersMapped[context.currentImage], &material,
         sizeof(material));
}
