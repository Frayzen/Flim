#pragma once

#include <fwd.hh>
#include <vulkan/vulkan_core.h>

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, Buffer &buffer);

VkCommandBuffer beginSingleTimeCommands();

void endSingleTimeCommands(VkCommandBuffer commandBuffer);

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void createBufferFromData(
    Buffer &buffer, VkBufferUsageFlags usage, void *data, size_t dataSize,
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

void *createMappedBuffer(
    Buffer &buffer, VkBufferUsageFlags usage, size_t bufSize,
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

void destroyBuffer(Buffer &buffer);
