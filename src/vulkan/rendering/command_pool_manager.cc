#include "command_pool_manager.hh"
#include "consts.hh"
#include "vulkan/device/device_utils.hh"
#include "vulkan/rendering/pipeline_manager.hh"

#include <cstdint>
#include <iostream>
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

void CommandPoolManager::recordCommandBuffer(VkCommandBuffer commandBuffer,
                                             uint32_t imageIndex) {

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;                  // Optional
  beginInfo.pInheritanceInfo = nullptr; // Optional

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo renderPassInfo{};

  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = context.pipeline.renderPass;
  renderPassInfo.framebuffer =
      context.swapChain.swapChainFramebuffers[imageIndex];
  // renderArea defines where shader loads and stores will take place
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = context.swapChain.swapChainExtent;

  // define the clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR, which we
  // used as load operation for the color attachment
  std::array<VkClearValue, 2> clearValues{};

  // The order of clearValues should be identical to the order of your
  // attachments
  clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    context.pipeline.graphicsPipeline);
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

  VkBuffer vertexBuffers[] = {context.vertexBuffer.buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer, context.indexBuffer.buffer, 0,
                       VK_INDEX_TYPE_UINT16);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          context.pipeline.pipelineLayout, 0, 1,
                          &context.descriptorSets[context.currentImage], 0,
                          nullptr);
  vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0,
                   0, 0);

  /*
    TAKES AS PARAMETER:
    * vertexCount: Even though we don't have a vertex buffer, we technically
    still have 3 vertices to draw.
    * instanceCount: Used for instanced rendering, use 1 if you're not doing
    that.
    * firstVertex: Used as an offset into the vertex buffer, defines the lowest
    value of gl_VertexIndex.
    * firstInstance: Used as an offset for instanced rendering, defines the
    lowest value of gl_InstanceIndex
  */
  /* vkCmdDraw(commandBuffer, 3, 1, 0, 0); */
  vkCmdEndRenderPass(commandBuffer);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
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
  return false;
}

bool CommandPoolManager::renderFrame(bool framebufferResized) {
  auto &imageAvailableSemaphores = commandPool.imageAvailableSemaphores;
  auto &renderFinishedSemaphores = commandPool.renderFinishedSemaphores;
  auto &inFlightFences = commandPool.inFlightFences;
  auto &commandBuffers = commandPool.commandBuffers;
  // Only reset the fence if we are submitting work
  vkResetFences(context.device, 1, &inFlightFences[context.currentImage]);

  vkResetCommandBuffer(commandBuffers[context.currentImage], 0);
  recordCommandBuffer(commandBuffers[context.currentImage], imageIndex);

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
