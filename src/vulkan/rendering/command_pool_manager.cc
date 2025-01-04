#include "command_pool_manager.hh"
#include "api/parameters.hh"
#include "api/render/mesh.hh"
#include "consts.hh"
#include "vulkan/device/device_utils.hh"
#include "vulkan/rendering/renderer.hh"

#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

void CommandPoolManager::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices =
      findQueueFamilies(context, context.physicalDevice);

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  // Allow command buffers to be rerecorded individually, without this flag they
  // all have to be reset together
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  if (vkCreateCommandPool(context.device, &poolInfo, nullptr,
                          &commandPool.pool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
}

static void createImageMemoryBarrier(const VkCommandBuffer &commandBuffer,
                                     VkImageLayout from, VkImageLayout to,
                                     const VkImage &image) {
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = from;
  barrier.newLayout = to;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  // Define access masks
  barrier.srcAccessMask =
      VK_ACCESS_NONE_KHR; // For VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  vkCmdPipelineBarrier(
      commandBuffer,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // Source stage
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // Destination stage
      0,                                             // Flags
      0, nullptr,                                    // Memory barriers
      0, nullptr,                                    // Buffer memory barriers
      1, &barrier                                    // Image memory barrier
  );
}

void CommandPoolManager::recordCommandBuffer(const Renderer &renderer) {
  VkCommandBuffer commandBuffer =
      commandPool.commandBuffers[context.currentImage];
  // Draw calls here
  Flim::Mesh &mesh = renderer.mesh;

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    renderer.pipeline.pipeline);

  std::vector<VkBuffer> vertexBuffers;
  std::vector<VkDeviceSize> offsets;
  for (auto &attr : renderer.params.getAttributeDescriptors()) {
    vertexBuffers.push_back(renderer.attributes.find(attr->id)->second.buffer);
    offsets.push_back(0);
  }

  vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(),
                         vertexBuffers.data(), offsets.data());
  vkCmdBindIndexBuffer(commandBuffer, renderer.indexBuffer.buffer, 0,
                       VK_INDEX_TYPE_UINT16);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          renderer.pipeline.pipelineLayout, 0, 1,
                          &renderer.descriptorSets[context.currentImage], 0,
                          nullptr);
  uint32_t nbIndices = static_cast<uint32_t>(mesh.indices.size());
  vkCmdDrawIndexed(commandBuffer, nbIndices, renderer.mesh.instances.size(), 0,
                   0, 0);
}

void CommandPoolManager::createCommandBuffers() {
  commandPool.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool.pool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)commandPool.commandBuffers.size();

  if (vkAllocateCommandBuffers(context.device, &allocInfo,
                               commandPool.commandBuffers.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void CommandPoolManager::createSyncObjects() {
  auto &imageAvailableSemaphores = commandPool.imageAvailableSemaphores;
  auto &renderFinishedSemaphores = commandPool.renderFinishedSemaphores;
  auto &inFlightFences = commandPool.inFlightFences;

  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  auto device = context.device;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                          &imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                          &renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) !=
            VK_SUCCESS) {

      throw std::runtime_error(
          "failed to create synchronization objects for a frame!");
    }
  }
}

static uint32_t imageIndex;

bool CommandPoolManager::acquireFrame() {
  auto &device = context.device;
  vkWaitForFences(device, 1, &commandPool.inFlightFences[context.currentImage],
                  VK_TRUE, UINT64_MAX);
  VkResult result = vkAcquireNextImageKHR(
      device, context.swapChain.swapChain, UINT64_MAX,
      commandPool.imageAvailableSemaphores[context.currentImage],
      VK_NULL_HANDLE, &imageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR)
    return true;
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    throw std::runtime_error("failed to acquire swap chain image!");
  auto &inFlightFences = commandPool.inFlightFences;
  // Only reset the fence if we are submitting work
  VkCommandBuffer &commandBuffer =
      commandPool.commandBuffers[context.currentImage];
  vkResetFences(context.device, 1, &inFlightFences[context.currentImage]);
  vkResetCommandBuffer(commandBuffer, 0);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;                  // Optional
  beginInfo.pInheritanceInfo = nullptr; // Optional

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  createImageMemoryBarrier(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           context.swapChain.swapChainImages[imageIndex]);

  VkRenderingAttachmentInfo depthInfo{};
  depthInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  depthInfo.imageView = context.depthImage.view;
  depthInfo.imageLayout = context.depthImage.layout;
  depthInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depthInfo.clearValue.depthStencil.depth = 1.0f;

  VkRenderingAttachmentInfoKHR attachmentInfoKHR{};
  attachmentInfoKHR.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  attachmentInfoKHR.imageView =
      context.swapChain.swapChainImageViews[imageIndex];
  attachmentInfoKHR.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attachmentInfoKHR.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachmentInfoKHR.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  // define the clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR, which we
  // used as load operation for the color attachment
  attachmentInfoKHR.clearValue = {{0, 0, 0, 1.0f}};

  VkRenderingInfoKHR renderInfo{};
  renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
  renderInfo.renderArea.offset = {0, 0};
  renderInfo.renderArea.extent = context.swapChain.swapChainExtent;
  renderInfo.layerCount = 1;
  renderInfo.colorAttachmentCount = 1;
  renderInfo.pColorAttachments = &attachmentInfoKHR;
  renderInfo.pDepthAttachment = &depthInfo;
  auto vkCmdBeginRenderingKHR =
      (PFN_vkCmdBeginRenderingKHR)vkGetInstanceProcAddr(
          context.instance, "vkCmdBeginRenderingKHR");
  vkCmdBeginRenderingKHR(commandBuffer, &renderInfo);

  // viewport and scissor state for this pipeline are dynamic
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(context.swapChain.swapChainExtent.width);
  viewport.height =
      static_cast<float>(context.swapChain.swapChainExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = context.swapChain.swapChainExtent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  return false;
}

bool CommandPoolManager::submitFrame(bool framebufferResized) {

  auto &imageAvailableSemaphores = commandPool.imageAvailableSemaphores;
  auto &renderFinishedSemaphores = commandPool.renderFinishedSemaphores;
  auto &inFlightFences = commandPool.inFlightFences;
  auto &commandBuffers = commandPool.commandBuffers;

  auto &commandBuffer = commandBuffers[context.currentImage];
  auto vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetInstanceProcAddr(
      context.instance, "vkCmdEndRenderingKHR");
  vkCmdEndRenderingKHR(commandBuffer);

  createImageMemoryBarrier(commandBuffer,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                           context.swapChain.swapChainImages[imageIndex]);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {
      imageAvailableSemaphores[context.currentImage]};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[context.currentImage];

  VkSemaphore signalSemaphores[] = {
      renderFinishedSemaphores[context.currentImage]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;
  if (vkQueueSubmit(context.queues.graphicsQueue, 1, &submitInfo,
                    inFlightFences[context.currentImage]) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }
  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  // specify which semaphores to wait on before presentation can hcontext
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {context.swapChain.swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr; // Optional
  VkResult result =
      vkQueuePresentKHR(context.queues.presentQueue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      framebufferResized)
    return true;
  if (result != VK_SUCCESS)
    throw std::runtime_error("failed to acquire swap chain image!");
  context.currentImage = (context.currentImage + 1) % MAX_FRAMES_IN_FLIGHT;
  return false;
}

void CommandPoolManager::cleanup() {
  auto device = context.device;
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(device, commandPool.renderFinishedSemaphores[i],
                       nullptr);
    vkDestroySemaphore(device, commandPool.imageAvailableSemaphores[i],
                       nullptr);
    vkDestroyFence(device, commandPool.inFlightFences[i], nullptr);
  }
  vkDestroyCommandPool(context.device, commandPool.pool, nullptr);
}
