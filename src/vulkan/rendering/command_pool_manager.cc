#include "command_pool_manager.hh"
#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "consts.hh"
#include "vulkan/context.hh"
#include "vulkan/device/device_utils.hh"
#include "vulkan/rendering/renderer.hh"

#include <cstdint>
#include <optional>
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
  poolInfo.queueFamilyIndex =
      queueFamilyIndices.graphicsAndComputeFamily.value();

  if (vkCreateCommandPool(context.device, &poolInfo, nullptr,
                          &commandPool.pool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
}

static VkAccessFlags getAccessMask(VkImageLayout layout) {
  // from: VK_IMAGE_LAYOUT_UNDEFINED, to
  // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL from:
  // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, to
  // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  switch (layout) {
  case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  case VK_IMAGE_LAYOUT_UNDEFINED:
    return VK_ACCESS_TRANSFER_READ_BIT;
  case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    return VK_ACCESS_MEMORY_READ_BIT;
  default:
    throw std::runtime_error("unexpected layout");
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

  if (to == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  } else {
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  }

  VkPipelineStageFlagBits srcstage, dststage;
  if (to == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
    srcstage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dststage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  } else {
    srcstage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dststage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  }

  vkCmdPipelineBarrier(commandBuffer,
                       srcstage,   // Source stage
                       dststage,   // Destination stage
                       0,          // Flags
                       0, nullptr, // Memory barriers
                       0, nullptr, // Buffer memory barriers
                       1, &barrier // Image memory barrier
  );
}

void CommandPoolManager::recordCommandBuffer(const Computer &computer) {
  VkCommandBuffer computeBuffer =
      commandPool.computeBuffers[context.currentImage];
  vkCmdBindPipeline(computeBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    computer.pipeline);
  vkCmdBindDescriptorSets(computeBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          computer.pipelineLayout, 0, 1,
                          &computer.descriptorSets[context.currentImage], 0, 0);
  vkCmdDispatch(computeBuffer, computer.dispatchAmount.x(),
                computer.dispatchAmount.y(), computer.dispatchAmount.z());
}

void CommandPoolManager::recordCommandBuffer(const Renderer &renderer) {
  VkCommandBuffer graphicBuffer =
      commandPool.graphicBuffers[context.currentImage];
  // Draw calls here
  const Flim::Mesh &mesh = renderer.mesh;

  vkCmdBindPipeline(graphicBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    renderer.pipeline.pipeline);

  std::vector<VkBuffer> vertexBuffers;
  std::vector<VkDeviceSize> offsets;
  for (auto &attr : renderer.params.getAttributeDescriptors()) {
    const std::shared_ptr<Flim::BaseAttributeDescriptor> &desc = attr.second;
    vertexBuffers.push_back(desc->getBuffer()->getVkBuffer());
    offsets.push_back(0);
  }

  vkCmdBindVertexBuffers(graphicBuffer, 0, vertexBuffers.size(),
                         vertexBuffers.data(), offsets.data());
  vkCmdBindIndexBuffer(graphicBuffer, renderer.indexBuffer.getVkBuffer(), 0,
                       VK_INDEX_TYPE_UINT16);
  vkCmdBindDescriptorSets(graphicBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          renderer.pipeline.pipelineLayout, 0, 1,
                          &renderer.descriptorSets[context.currentImage], 0,
                          nullptr);
  uint32_t nbIndices = static_cast<uint32_t>(mesh.indices.size());
  vkCmdDrawIndexed(graphicBuffer, nbIndices, renderer.mesh.instances.size(), 0,
                   0, 0);
}

void CommandPoolManager::createCommandBuffer(
    std::vector<VkCommandBuffer> &buffers) {
  buffers.resize(MAX_FRAMES_IN_FLIGHT);
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool.pool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)commandPool.graphicBuffers.size();
  if (vkAllocateCommandBuffers(context.device, &allocInfo, buffers.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate graphic buffers!");
  }
}

void CommandPoolManager::createCommandBuffers() {
  createCommandBuffer(commandPool.graphicBuffers);
  createCommandBuffer(commandPool.computeBuffers);
}

void CommandPoolManager::createSyncObjects() {
  auto &imageAvailableSemaphores = commandPool.imageAvailableSemaphores;
  auto &renderFinishedSemaphores = commandPool.renderFinishedSemaphores;
  auto &inFlightFences = commandPool.inFlightFences;

  auto &computeFinishedSemaphores = commandPool.computeFinishedSemaphores;
  auto &computeInFlightFences = commandPool.computeInFlightFences;

  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  computeInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  computeFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

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
            VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                          &computeFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &computeInFlightFences[i]) !=
            VK_SUCCESS) {

      throw std::runtime_error(
          "failed to create synchronization objects for a frame!");
    }
  }
}

static uint32_t imageIndex;

static void beginCmdBuffer(VkCommandBuffer &cmdBuffer) {
  vkResetCommandBuffer(cmdBuffer, 0);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;                  // Optional
  beginInfo.pInheritanceInfo = nullptr; // Optional

  if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  createImageMemoryBarrier(cmdBuffer, VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           context.swapChain.swapChainImages[imageIndex]);
}

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
  auto &computeInFlightFences = commandPool.computeInFlightFences;

  // Only reset the fence if we are submitting work
  vkResetFences(context.device, 1, &inFlightFences[context.currentImage]);
  vkResetFences(context.device, 1,
                &computeInFlightFences[context.currentImage]);

  VkCommandBuffer &graphicBuffer =
      commandPool.graphicBuffers[context.currentImage];
  beginCmdBuffer(graphicBuffer);

  VkCommandBuffer &computeBuffer =
      commandPool.computeBuffers[context.currentImage];
  beginCmdBuffer(computeBuffer);

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
  vkCmdBeginRenderingKHR(graphicBuffer, &renderInfo);

  // viewport and scissor state for this pipeline are dynamic
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(context.swapChain.swapChainExtent.width);
  viewport.height =
      static_cast<float>(context.swapChain.swapChainExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(graphicBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = context.swapChain.swapChainExtent;
  vkCmdSetScissor(graphicBuffer, 0, 1, &scissor);
  return false;
}

static void endCmdBuffer(
    VkCommandBuffer &cmdBuffer, VkQueue &queue,
    std::optional<std::vector<VkSemaphore>> waitSemaphore = std::nullopt,
    std::optional<VkSemaphore> signalSemaphore = std::nullopt,
    std::optional<VkFence> fence = std::nullopt) {
  if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // TODO handle that in parameters
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT};
  if (waitSemaphore.has_value()) {
    auto &wsems = waitSemaphore.value();
    submitInfo.waitSemaphoreCount = wsems.size();
    submitInfo.pWaitSemaphores = wsems.data();
    submitInfo.pWaitDstStageMask = waitStages;
  }
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmdBuffer;

  if (signalSemaphore.has_value()) {
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSemaphore.value();
  }

  if (vkQueueSubmit(queue, 1, &submitInfo, fence.value_or(nullptr)) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }
}

bool CommandPoolManager::submitFrame(bool framebufferResized) {
  auto &imageAvailableSemaphores = commandPool.imageAvailableSemaphores;
  auto &renderFinishedSemaphores = commandPool.renderFinishedSemaphores;
  auto &inFlightFences = commandPool.inFlightFences;
  auto &graphicsBuffer = commandPool.graphicBuffers[context.currentImage];

  auto &computeFinishedSemaphores = commandPool.computeFinishedSemaphores;
  auto &computeInFlightFences = commandPool.computeInFlightFences;
  auto &computeBuffer = commandPool.computeBuffers[context.currentImage];

  // COMPUTE QUEUE
  endCmdBuffer(computeBuffer, context.queues.computeQueue, std::nullopt,
               computeFinishedSemaphores[context.currentImage],
               computeInFlightFences[context.currentImage]);

  // GRAPHIC QUEUE
  auto vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetInstanceProcAddr(
      context.instance, "vkCmdEndRenderingKHR");
  vkCmdEndRenderingKHR(graphicsBuffer);

  createImageMemoryBarrier(graphicsBuffer,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                           context.swapChain.swapChainImages[imageIndex]);
  std::vector<VkSemaphore> waitSemaphores = {
      imageAvailableSemaphores[context.currentImage],
      computeFinishedSemaphores[context.currentImage],
  };
  endCmdBuffer(graphicsBuffer, context.queues.graphicsQueue, waitSemaphores,
               renderFinishedSemaphores[context.currentImage],
               inFlightFences[context.currentImage]);

  // PRESENT

  vkDeviceWaitIdle(context.device);
  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  // specify which semaphores to wait on before presentation can hcontext
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderFinishedSemaphores[context.currentImage];

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
  context.currentUpdate = (context.currentImage + 1) % MAX_FRAMES_IN_FLIGHT;
  return false;
}

void CommandPoolManager::cleanup() {
  auto device = context.device;
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    // Graphic
    vkDestroySemaphore(device, commandPool.renderFinishedSemaphores[i],
                       nullptr);
    vkDestroySemaphore(device, commandPool.imageAvailableSemaphores[i],
                       nullptr);
    vkDestroyFence(device, commandPool.inFlightFences[i], nullptr);

    // Compute
    vkDestroySemaphore(device, commandPool.computeFinishedSemaphores[i],
                       nullptr);
    vkDestroyFence(device, commandPool.computeInFlightFences[i], nullptr);
  }
  vkDestroyCommandPool(context.device, commandPool.pool, nullptr);
}
