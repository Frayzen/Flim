#pragma once

#include <fwd.hh>
#include <vulkan/vulkan_core.h>

class Buffer {
public:
  Buffer(void *ptr, int size, VkBufferUsageFlags usage = 0,
         VkMemoryPropertyFlags properties = 0)
      : Buffer(size) {
    create(VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | properties);
    populate(ptr);
  };

  Buffer(int size)
      : size(size), created(false), mappedPtr(nullptr), buffer(0) {};

  const VkDeviceMemory &getVkBufferMemory() const { return bufferMemory; };
  const VkBuffer &getVkBuffer() const { return buffer; };
  void *getPtr() const { return mappedPtr; };
  void map();
  void unmap();

  void copy(const Buffer &from) const;
  void populate(void *value) const;

  void create(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
  void destroy();

private:
  VkBufferUsageFlags usage;
  VkMemoryPropertyFlags properties;

  bool created;
  int size; // in bytes
  VkDeviceMemory bufferMemory;
  VkBuffer buffer;
  void *mappedPtr;
};

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

VkCommandBuffer beginSingleTimeCommands();

void endSingleTimeCommands(VkCommandBuffer commandBuffer);
