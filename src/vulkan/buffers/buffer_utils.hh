#pragma once

#include <fwd.hh>
#include <vulkan/vulkan_core.h>

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, Buffer &buffer);

VkCommandBuffer beginSingleTimeCommands();

void endSingleTimeCommands(VkCommandBuffer commandBuffer);

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void populateBufferFromData(Buffer &buffer, VkBufferUsageFlags usage,
                            void *data, size_t dataSize);

void destroyBuffer(Buffer &buffer);
