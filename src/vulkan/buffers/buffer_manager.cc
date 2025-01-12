#include "buffer_manager.hh"
#include "consts.hh"
#include "vulkan/context.hh"
#include <iostream>
#include <memory>

BufferManager &bufferManager = BufferManager::get();

// Manager

BufferManager &BufferManager::get() {
  static BufferManager instance;
  return instance;
}
int BufferManager::createId() const {
  static int cur = 0;
  return cur++;
}
void BufferManager::attachMesh(int bufferId, Flim::Mesh *mesh) {
  assert(attachedMesh[bufferId] == nullptr);
  std::cout << "SET " << bufferId << " TO " << mesh << std::endl;
  attachedMesh[bufferId] = mesh;
}

// Holder
BufferHolder::BufferHolder(const BufferHolder &other)
    : bufferId(bufferManager.createId()), redundancy(other.redundancy) {}

BufferHolder::BufferHolder(int id)
    : bufferId(id), redundancy(MAX_FRAMES_IN_FLIGHT) {}

BufferHolder::BufferHolder() : BufferHolder(bufferManager.createId()) {}

void BufferHolder::setupBuffers(int bufferSize, VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties) {
  std::cout << "TRYING SETUP " << bufferId << std::endl;
  if (bufferManager.buffers.contains(bufferId))
    return;
  std::cout << "SUCCES" << '\n';
  bufferManager.buffers[bufferId] =
      std::vector<std::shared_ptr<Buffer>>(redundancy);
  if (!bufferManager.attachedMesh.contains(bufferId))
    bufferManager.attachedMesh[bufferId] = nullptr;
  for (int i = 0; i < redundancy; i++)
    bufferManager.buffers[bufferId][i] = std::make_shared<Buffer>(bufferSize);
  for (auto b : bufferManager.buffers[bufferId])
    b->create(usage, properties);
}
void BufferHolder::cleanupBuffers() {
  if (!bufferManager.buffers.contains(bufferId))
    return;
  for (auto &b : bufferManager.buffers[bufferId])
    b->destroy();
  bufferManager.buffers[bufferId].clear();
}

std::vector<std::shared_ptr<Buffer>> &BufferHolder::getBuffers() const {
  return bufferManager.buffers[bufferId];
}

std::shared_ptr<Buffer> BufferHolder::getBuffer(int i) const {
  int cur = i;
  if (i == -1)
    cur = context.currentImage;
  cur %= redundancy;
  return bufferManager.buffers[bufferId][cur];
}

int BufferHolder::getBufferId() const { return bufferId; }

Flim::Mesh *BufferHolder::getAttachedMesh() const {
  return bufferManager.attachedMesh[bufferId];
}
