#include "buffer_manager.hh"
#include <cstddef>
void BufferManager::cleanup() {
  for (auto &image : context.images) {
    vkDestroySampler(context.device, image.sampler, nullptr);
    vkDestroyImageView(context.device, image.view, nullptr);
    vkDestroyImage(context.device, image.textureImage, nullptr);
    vkFreeMemory(context.device, image.textureImageMemory, nullptr);
  }
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(context.device, uniformLocationBuffers[i].buffer, nullptr);
    vkFreeMemory(context.device, uniformLocationBuffers[i].bufferMemory, nullptr);

    vkDestroyBuffer(context.device, uniformMaterialBuffers[i].buffer, nullptr);
    vkFreeMemory(context.device, uniformMaterialBuffers[i].bufferMemory, nullptr);
  }
  vkDestroyDescriptorPool(context.device, context.descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(context.device, context.descriptorSetLayout,
                               nullptr);

  vkDestroyBuffer(context.device, context.indexBuffer.buffer, nullptr);
  vkFreeMemory(context.device, context.indexBuffer.bufferMemory, nullptr);

  vkDestroyBuffer(context.device, context.vertexBuffer.buffer, nullptr);
  vkFreeMemory(context.device, context.vertexBuffer.bufferMemory, nullptr);
}
