#pragma once

#include "api/scene.hh"
#include "vulkan/app.hh"
#include <GLFW/glfw3.h>
#include <functional>

namespace Flim {

class FlimAPI {

public:
  static FlimAPI init();
  int run(const std::function<void(float)> &renderMethod);
  int run();
  Scene &getScene();

  GLFWwindow *getWindow();
  void cleanup();
private:
  Scene scene;
  VulkanApplication app;
  FlimAPI() : scene(*this) {};
};

} // namespace Flim
