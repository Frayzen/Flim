#pragma once

#include "api/fwd.hh"
#include "api/parameters.hh"
#include "api/render/mesh.hh"
#include "api/tree/camera_object.hh"
#include "api/tree/instance_object.hh"
#include "api/tree/root_object.hh"
#include "api/tree/tree_object.hh"

namespace Flim {

extern Renderer emptyRenderer;

class Scene {

public:
  void defaultRenderer(Renderer &renderer);

  InstanceObject &instantiate(Mesh &mesh);
  const RootObject &getRoot();

  FlimAPI &api;
  Renderer &renderer;
  CameraObject *mainCamera;
  void invalidateRenderer();

private:
  bool invalidatedRenderer;
  RootObject root;
  Scene(FlimAPI &api)
      : api(api), root(*this), mainCamera(), renderer(emptyRenderer), invalidatedRenderer(false) {
    mainCamera = &root.append<CameraObject>();
  };
  friend class FlimAPI;
};

} // namespace Flim
