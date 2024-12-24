#pragma once

#include "api/fwd.hh"
#include "api/parameters.hh"
#include "api/render/mesh.hh"
#include "api/tree/free_camera_object.hh"
#include "api/tree/instance_object.hh"
#include "api/tree/root_object.hh"
#include "api/tree/tree_object.hh"

namespace Flim {

class Scene {

public:
  void defaultRenderer(Renderer *renderer);

  InstanceObject &instantiate(Mesh &mesh);
  const RootObject &getRoot();

  FlimAPI &api;
  Renderer *renderer;
  FreeCameraObject *mainCamera;
  void invalidateRenderer();

private:
  bool invalidatedRenderer;
  RootObject root;
  Scene(FlimAPI &api)
      : api(api), root(*this), mainCamera(), invalidatedRenderer(false) {
    mainCamera = &root.append<FreeCameraObject>();
  };
  friend class FlimAPI;
};

} // namespace Flim
