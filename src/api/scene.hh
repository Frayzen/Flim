#pragma once

#include "api/fwd.hh"
#include "api/parameters.hh"
#include "api/render/mesh.hh"
#include "api/tree/camera_object.hh"
#include "api/tree/root_object.hh"
#include "api/tree/tree_object.hh"

namespace Flim {

class Scene {

public:
  void defaultRenderer(Renderer *renderer);
  TreeObject instantiate(Mesh &mesh);

  Renderer *renderer;
  CameraObject mainCamera;

private:
  RootObject root;
  Scene() : mainCamera(&root, *this), root(*this) {};
  friend class FlimAPI;
};

} // namespace Flim
