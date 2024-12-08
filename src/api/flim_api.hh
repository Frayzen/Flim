#pragma once

#include "api/parameters.hh"
#include "api/scene.hh"
#include "vulkan/app.hh"

namespace Flim {

class FlimAPI {


public:
  static FlimAPI init(FlimParameters parameters = {});
  int run();
  ~FlimAPI();
  Scene& getScene();

private:
  Scene scene;
  VulkanApplication app;
  FlimAPI() = default;
};

} // namespace Flim
