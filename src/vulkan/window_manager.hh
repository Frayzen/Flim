#pragma once

#include "vulkan/base_manager.hh"
#include "vulkan/context.hh"
#include <GLFW/glfw3.h>
class WindowManager : public BaseManager {
public:
  WindowManager(VulkanContext &context) : BaseManager(context) {};
  void initWindow();
  bool framebufferResized = false;
};
