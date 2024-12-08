#pragma once

#include "api/fwd.hh"
#include "api/parameters.hh"
#include "api/render/mesh.hh"
#include "api/tree/camera_object.hh"
#include "api/tree/instance_object.hh"
#include "api/tree/root_object.hh"

namespace Flim {

class Scene {

public:
  void defaultRenderer(Renderer *renderer);
  InstanceObject& instantiate(Mesh &mesh);
  const RootObject &getRoot();

  Renderer *renderer;
  CameraObject* mainCamera;

private:
  RootObject root;
  Scene() : root(*this), mainCamera(nullptr) {};
  friend class FlimAPI;
};

} // namespace Flim
