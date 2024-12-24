#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <vulkan/vulkan_core.h>

class GUIManager {
public:
  GUIManager() = default;
  void setup();
  void cleanup();
  void beginFrame();
  void endFrame();

private:
  VkCommandBuffer guiCommandBuffer;
  ImGuiContext *imGuiContext;
  VkDescriptorPool imguiPool;
};
