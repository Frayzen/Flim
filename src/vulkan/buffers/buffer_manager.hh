#pragma once

#include "api/render/mesh.hh"
#include "api/tree/free_camera_object.hh"
#include "vulkan/base_manager.hh"
#include "vulkan/context.hh"
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

struct UniformLocationObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

struct UniformMaterialObject {
  alignas(16) glm::vec3 ambient;
  alignas(16) glm::vec3 diffuse;
  alignas(16) glm::vec3 specular;
};

class BufferManager : public BaseManager {
public:
  BufferManager(VulkanContext &context) : BaseManager(context) {};

  // Vertices
  void createVertexBuffer(const std::vector<Flim::Vertex> &vertices);
  void createIndexBuffer(const std::vector<uint16> indices);
  void createDescriptorSetLayout();
  void createUniformBuffers();

  // Uniform
  void updateUniformBuffer(const Flim::Mesh &mesh,
                           const Flim::FreeCameraObject *cam);
  void createDescriptorPool();
  void createDescriptorSets();

  // Texture
  void createTextureImage();
  void createTextureImageView();
  void createTextureSampler();
  void createDepthResources();

  void cleanup();

private:
  std::vector<Buffer> uniformLocationBuffers;
  std::vector<void *> uniformLocationBuffersMapped;

  std::vector<Buffer> uniformMaterialBuffers;
  std::vector<void *> uniformMaterialBuffersMapped;

  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);

  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);

  // Texture
  void createImage(Image &image, VkFormat format, VkImageTiling tiling,
                   VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
  void copyBufferToImage(VkBuffer buffer, Image &image);
  void transitionImageLayout(Image &image, VkImageLayout newLayout);
  void createImageView(Image &image, VkImageAspectFlags aspectFlags);

  // Buffer
  void createBuffer(VulkanContext &context, VkDeviceSize size,
                    VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                    Buffer &buffer);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};
