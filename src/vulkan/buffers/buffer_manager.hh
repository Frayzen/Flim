#pragma once

#include "api/render/mesh.hh"
#include "buffer_utils.hh"
#include <cwchar>
#include <fwd.hh>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace Flim {
class RenderParams;
class ComputeParams;
}; // namespace Flim

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
  void attachMesh(int bufferId, Flim::Mesh *mesh);

  int createId() const;
  static BufferManager &get();

private:
  BufferManager() = default;

  std::map<int, std::vector<std::shared_ptr<Buffer>>> buffers;
  std::map<int, Flim::Mesh *> attachedMesh;
  friend class BufferHolder;
};

extern BufferManager &bufferManager;

/*
 * A generic holder of underlying data aimed to be extended by other classes.
 * It generates the Buffer objects (with possible duplication in case of
 * per-frame-buffer)
 * Usefull for buffer per frame Redundancy is the total amount
 * of duplicate buffer of the same content
 */
class BufferHolder {
public:
  BufferHolder(const BufferHolder &);
  std::shared_ptr<Buffer> getBuffer(int i = -1) const;
  int getBufferId() const;
  int newBufferId() const;
  Flim::Mesh *getAttachedMesh() const;

  int bufferId; // tmp, to put back in protected
protected:
  BufferHolder();
  BufferHolder(int id);
  std::vector<std::shared_ptr<Buffer>> &getBuffers() const;
  void setupBuffers(std::string name, int bufferSize, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, bool computeFriendly);
  ~BufferHolder();

  int redundancy;

  friend class Flim::RenderParams;
  friend class Flim::ComputeParams;
};
