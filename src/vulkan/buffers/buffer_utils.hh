#pragma once

#include <Kokkos_Core.hpp>
#include <fwd.hh>
#include <vulkan/vulkan_core.h>

class Buffer {
public:
  Buffer(void *ptr, int size, VkBufferUsageFlags usage = 0,
         VkMemoryPropertyFlags properties = 0, bool external = true)
      : Buffer(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
               properties | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               external) {
    populate(ptr);
  };

  Buffer(int size,
         VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VkMemoryPropertyFlags properties = 0, bool external = true)
      : size(size), created(false), mappedPtr(nullptr), buffer(0),
        external(external), externalPtr(nullptr) {
    create(usage, properties);
  };

  const VkDeviceMemory &getVkBufferMemory() const { return bufferMemory; };
  const VkBuffer &getVkBuffer() const { return buffer; };
  void *getPtr() const { return mappedPtr; };
  int getSize() const { return size; };
  template <typename Type>
  Kokkos::View<Type *, Kokkos::DefaultExecutionSpace> getView() const {
    // Check if the DefaultExecutionSpace can access a device memory space
    using HostMemSpace = typename Kokkos::HostSpace::memory_space;
    static_assert(!Kokkos::SpaceAccessibility<Kokkos::DefaultExecutionSpace,
                                              HostMemSpace>::accessible);
    assert(size % sizeof(Type) == 0);
    return Kokkos::View<Type *, Kokkos::DefaultExecutionSpace>(
        (Type *)externalPtr, size / sizeof(Type));
  }

  void map();
  void unmap();

  void copy(const Buffer &from) const;
  void populate(void *value) const;

  void destroy();

private:
  void create(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
  bool created;
  int size; // in bytes
  VkDeviceMemory bufferMemory;
  VkBuffer buffer;
  void *mappedPtr;

  bool external;
  void *externalPtr;
  int externalFd;
};

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

VkCommandBuffer beginSingleTimeCommands();

void endSingleTimeCommands(VkCommandBuffer commandBuffer);
