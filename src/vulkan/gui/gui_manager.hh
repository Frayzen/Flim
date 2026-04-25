#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <vulkan/vulkan_core.h>
namespace Flim {

class GUIManager {
public:
  GUIManager() = default;
  ~GUIManager();
  void setup();
  void beginFrame();
  void endFrame();

private:
  VkCommandBuffer guiCommandBuffer;
  ImGuiContext *imGuiContext;
  VkDescriptorPool imguiPool;
};
} // namespace Flim
