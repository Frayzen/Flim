#pragma once

#include "api/render/mesh.hh"
#include "buffer_utils.hh"
#include <cwchar>
#include <fwd.hh>
#include <memory>
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
  void attachMesh(int bufferId, Flim::Mesh* mesh);

  int createId() const;
  static BufferManager &get();

private:
  BufferManager() = default;

  std::map<int, std::vector<std::shared_ptr<Buffer>>> buffers;
  std::map<int, Flim::Mesh*> attachedMesh;
  friend class BufferHolder;
};

extern BufferManager &bufferManager;

/*
 * Usefull for buffer per frame
 * redudancy is the total amount of duplicate buffer of the same content
 */
class BufferHolder {
public:
  std::shared_ptr<Buffer> getBuffer(int i = -1) const;
  int getBufferId() const;
  int newBufferId() const;
  Flim::Mesh* getAttachedMesh() const;

protected:

  BufferHolder(const BufferHolder &);
  BufferHolder();
  BufferHolder(int id);
  int redundancy;
  std::vector<std::shared_ptr<Buffer>> &getBuffers() const;
  void setupBuffers(int bufferSize, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties);
  void cleanupBuffers();
  int bufferId;
};
