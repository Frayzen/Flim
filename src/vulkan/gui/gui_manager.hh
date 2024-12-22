#pragma once

#include "vulkan/base_manager.hh"
#include "vulkan/context.hh"
#include <imgui.h>
#include <imgui_internal.h>

class GUIManager : public BaseManager {
public:
  GUIManager(VulkanContext &context) : BaseManager(context) {}
  void setup();
  void cleanup();
  void beginFrame();
  void endFrame();

private:
  VkCommandBuffer guiCommandBuffer;
  ImGuiContext *imGuiContext;
  VkDescriptorPool imguiPool;
};
