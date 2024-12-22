#include "gui_manager.hh"

#include <imconfig.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imstb_rectpack.h>
#include <imstb_textedit.h>
#include <imstb_truetype.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <stdexcept>
#include <vulkan/vulkan_core.h>

void GUIManager::setup() {
  // this initializes the core structures of imgui
  imGuiContext = ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  // this initializes imgui for SDL
  ImGui_ImplGlfw_InitForVulkan(context.window, true);

  // this initializes imgui for Vulkan
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = context.instance;
  init_info.PhysicalDevice = context.physicalDevice;
  init_info.Device = context.device;
  init_info.Queue = context.queues.graphicsQueue;
  init_info.RenderPass = context.pipeline.renderPass;
  init_info.DescriptorPoolSize = 1;
  init_info.MinImageCount = 3;
  init_info.ImageCount = 3;
  init_info.MinAllocationSize = 1024 * 1024;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

  ImGui_ImplVulkan_Init(&init_info);
  // Allocate a command buffer

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = context.commandPool.pool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer cb;
  if (vkAllocateCommandBuffers(context.device, &allocInfo, &cb) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cb, &beginInfo);
  ImGui_ImplVulkan_CreateFontsTexture();
  vkEndCommandBuffer(cb);

  // Submit the command buffer
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cb;

  vkQueueSubmit(context.queues.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(context.queues.graphicsQueue);

  ImGui_ImplVulkan_CreateFontsTexture();
  ImGui_ImplVulkan_DestroyFontsTexture();
}

void GUIManager::frameUpdate() {
  // Start a new ImGui frame
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // Add UI elements here
  ImGui::Button("Test");

  // Render ImGui
  ImGui::Render();

  // Submit the ImGui draw data to Vulkan
  auto &commandBuffer =
      context.commandPool.commandBuffers[context.currentImage];
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void GUIManager::cleanup() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext(imGuiContext);
}
