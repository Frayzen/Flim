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

int FlimAPI::run(const std::function<void()> &renderMethod) {
  assert(scene.renderer != nullptr);
  app.setupGraphics(scene);
  try {
    double lastTime = glfwGetTime();
    while (true) {
      if (app.mainLoop(renderMethod, scene))
        break;
      if (scene.invalidatedRenderer) {
        app.updateGraphics(scene);
        scene.invalidatedRenderer = false;
      }
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

GLFWwindow *FlimAPI::getWindow() { return context.window; }

} // namespace Flim
