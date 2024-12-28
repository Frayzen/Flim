#pragma once

#include "api/fwd.hh"
#include "api/parameters.hh"
#include "api/render/mesh.hh"
#include "api/tree/camera.hh"
#include "vulkan/rendering/renderer.hh"
#include <map>

class VulkanApplication;

namespace Flim {

class InstanceObject;

class Scene {

public:
  InstanceObject &instantiate(Mesh &mesh);

  void registerMesh(Mesh &mesh, RenderParams &params);
  FlimAPI &api;
  CameraObject mainCamera;
  std::map<int, std::shared_ptr<Renderer>> renderers; // key is mesh.id

private:
  Scene(FlimAPI &api) : api(api), mainCamera(*this) {};
  friend class FlimAPI;
  friend class ::VulkanApplication;
};

} // namespace Flim
