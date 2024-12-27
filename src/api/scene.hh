#pragma once

#include "api/fwd.hh"
#include "api/parameters.hh"
#include "api/render/mesh.hh"
#include "api/tree/camera_object.hh"
#include "api/tree/root_object.hh"
#include "api/tree/tree_object.hh"
#include "vulkan/rendering/renderer.hh"
#include <map>

class VulkanApplication;

namespace Flim {

class InstanceObject;

class Scene {

public:
  InstanceObject &instantiate(Mesh &mesh);
  const RootObject &getRoot();

  void registerMesh(Mesh &mesh, RenderParams &params);
  FlimAPI &api;
  CameraObject *mainCamera;
  std::map<int, std::shared_ptr<Renderer>> renderers; // key is mesh.id

private:
  RootObject root;
  Scene(FlimAPI &api) : api(api), root(*this), mainCamera() {
    mainCamera = &root.append<CameraObject>();
  };
  friend class FlimAPI;
  friend class ::VulkanApplication;
};

} // namespace Flim
