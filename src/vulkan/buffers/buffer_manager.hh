#pragma once

#include "vulkan/base_manager.hh"
#include "vulkan/context.hh"

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

class BufferManager : public BaseManager {
public:
  BufferManager(VulkanContext &context) : BaseManager(context) {};

  void createVertexBuffer();
  void createIndexBuffer();
  void createDescriptorSetLayout();
  void createUniformBuffers();

  // Uniform
  void updateUniformBuffer();
  void createDescriptorPool();
  void createDescriptorSets();

  void cleanup();

private:
  std::vector<Buffer> uniformBuffers;
  std::vector<void *> uniformBuffersMapped;

  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void createBuffer(VulkanContext &context, VkDeviceSize size,
                    VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                    Buffer &buffer);
};
