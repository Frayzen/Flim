#include "surface_manager.hh"
#include "vulkan/context_holder.hh"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

void SurfaceManager::createSurface() {
  VkResult err =
      glfwCreateWindowSurface(getContext().instance, getContext().window,
                              nullptr, &getContext().surface);
  if (err != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
}

