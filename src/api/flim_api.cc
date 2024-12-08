#include "flim_api.hh"
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
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

} // namespace Flim
