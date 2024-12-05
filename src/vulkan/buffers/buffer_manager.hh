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

  // Texture
  void createTextureImage();

  void cleanup();

private:
  std::vector<Buffer> uniformBuffers;
  std::vector<void *> uniformBuffersMapped;

  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);

  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);

  // Texture
  void createImage(Image &image, VkFormat format, VkImageTiling tiling,
                   VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
  void copyBufferToImage(VkBuffer buffer, Image &image);
  void transitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout);

  // Buffer
  void createBuffer(VulkanContext &context, VkDeviceSize size,
                    VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                    Buffer &buffer);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};
