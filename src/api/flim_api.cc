#include "flim_api.hh"
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>

namespace Flim {

FlimAPI FlimAPI::init(FlimParameters parameters) {
  (void)parameters;
  FlimAPI flim;
  flim.app.init();
  return flim;
}

Scene &FlimAPI::getScene() { return scene; }

FlimAPI::~FlimAPI() { app.cleanup(); }

int FlimAPI::run() {
  assert(scene.renderer != nullptr);
  app.setupGraphics(scene);
  try {
    double lastTime = glfwGetTime();
    while (true) {
      if (app.mainLoop(scene))
        break;
      double curTime = glfwGetTime();
      double deltaTime = lastTime - curTime;
      scene.mainCamera->handleInputs(deltaTime);
      lastTime = curTime;
    }
    app.finish();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

GLFWwindow *FlimAPI::getWindow() { return app.context.window; }

} // namespace Flim
