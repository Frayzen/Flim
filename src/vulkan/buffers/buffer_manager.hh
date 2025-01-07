#pragma once

#include "buffer_utils.hh"
#include <cwchar>
#include <fwd.hh>
#include <vulkan/vulkan_core.h>

struct UniformLocationObject {
  Matrix4f model;
  Matrix4f view;
  Matrix4f proj;
};

struct UniformMaterialObject {
  alignas(16) Vector3f ambient;
  alignas(16) Vector3f diffuse;
  alignas(16) Vector3f specular;
};

/*
 * Manage lifetime of ALL the buffers
 */
class BufferManager {
public:
  BufferManager(BufferManager &) = delete;

  void createDepthResources();
  int createId() const;
  static BufferManager &get();

private:
  BufferManager() = default;

  std::map<int, std::vector<Buffer>> buffers;
  friend class BufferHolder;
};

extern BufferManager &bufferManager;

/*
 * Usefull for buffer per frame
 * redudancy is the total amount of duplicate buffer of the same content
 */
class BufferHolder {
public:
  Buffer &getBuffer(int i = -1) const;

protected:
  BufferHolder();
  BufferHolder(int id);
  const int bufferId;
  int redundancy;
  std::vector<Buffer> &getBuffers() const;
  void setupBuffers(int bufferSize, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties);
  void cleanupBuffers();
};
