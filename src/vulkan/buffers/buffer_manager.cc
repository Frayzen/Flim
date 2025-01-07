#include "buffer_manager.hh"
#include "consts.hh"
#include "fwd.hh"
#include "vulkan/context.hh"

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

// Holder

BufferHolder::BufferHolder(int id)
    : bufferId(id), redundancy(MAX_FRAMES_IN_FLIGHT) {}
BufferHolder::BufferHolder() : BufferHolder(bufferManager.createId()) {}

void BufferHolder::setupBuffers(int bufferSize, VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties) {
  if (bufferManager.buffers.contains(bufferId))
    return;
  bufferManager.buffers[bufferId] = std::vector<Buffer>(redundancy, bufferSize);
  for (Buffer b : bufferManager.buffers[bufferId])
    b.create(usage, properties);
}
void BufferHolder::cleanupBuffers() {
  if (!bufferManager.buffers.contains(bufferId))
    return;
  for (auto &b : bufferManager.buffers[bufferId])
    b.destroy();
  bufferManager.buffers[bufferId].clear();
}

std::vector<Buffer> &BufferHolder::getBuffers() const {
  return bufferManager.buffers[bufferId];
}

Buffer &BufferHolder::getBuffer(int i) const {
  int cur = i;
  if (i == -1)
    cur = context.currentImage;
  cur %= redundancy;
  return bufferManager.buffers[bufferId][cur];
}
