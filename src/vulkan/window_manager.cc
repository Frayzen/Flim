#include "window_manager.hh"
#include <GLFW/glfw3.h>
#include <stdexcept>

static void framebufferResizeCallback(GLFWwindow *window, int width,
                                      int height) {
  auto windowManager =
      reinterpret_cast<WindowManager *>(glfwGetWindowUserPointer(window));
  windowManager->framebufferResized = true;
}

void WindowManager::initWindow() {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }
  if (!glfwVulkanSupported()) {
    throw std::runtime_error("Vulkan is not supported by GLFW");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable OpenGL (default API)
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // For later
  context.window = glfwCreateWindow(width, height, "FLIM", nullptr, nullptr);
  if (!context.window) {
    throw std::runtime_error("Failed to create GLFW window");
  }
  glfwSetWindowUserPointer(context.window, this);
  glfwSetFramebufferSizeCallback(context.window, framebufferResizeCallback);
  const char *desc;
  if (glfwGetError(&desc)) { // In case an error occured
    throw std::runtime_error(desc);
  }
}
