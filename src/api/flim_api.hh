#pragma once

#include "api/parameters.hh"
#include "api/scene.hh"
#include "vulkan/app.hh"
#include <GLFW/glfw3.h>
#include <functional>

namespace Flim {

class FlimAPI {

public:
  static FlimAPI init(FlimParameters parameters = {});
  int run(const std::function<void()> &renderMethod);
  int run();
  void cleanup();
  Scene &getScene();

  GLFWwindow *getWindow();

private:
  Scene scene;
  VulkanApplication app;
  FlimAPI() : scene(*this) {};
};

} // namespace Flim
