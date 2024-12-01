#include "window_manager.hh"
#include <stdexcept>

void WindowManager::initWindow() {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }
  if (!glfwVulkanSupported()) {
    throw std::runtime_error("Vulkan is not supported by GLFW");
  }
  glfwWindowHint(GLFW_CLIENT_API,
                 GLFW_NO_API);                // Disable OpenGL (default API)
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // For later
  context.window = glfwCreateWindow(width, height, "FLIM", nullptr, nullptr);
  if (!context.window) {
    throw std::runtime_error("Failed to create GLFW window");
  }
  const char *desc;
  if (glfwGetError(&desc)) { // In case an error occured
    throw std::runtime_error(desc);
  }
}
