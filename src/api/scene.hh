#pragma once

#include "api/fwd.hh"
#include "api/parameters/compute_params.hh"
#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/tree/camera.hh"
#include "vulkan/computing/computer.hh"
#include "vulkan/rendering/renderer.hh"
#include <map>

class VulkanApplication;

namespace Flim {

class Instance;

class Scene {

public:
  Instance &instantiate(Mesh &mesh) const;

  void registerMesh(Mesh &mesh, RenderParams &params);
  void registerMesh(Mesh &mesh, RenderParams &rparams, ComputeParams& cparams);
  FlimAPI &api;
  Camera camera;
  std::map<int, std::shared_ptr<Renderer>> renderers; // key is mesh.id
  std::map<int, std::shared_ptr<Computer>> computers; // key is mesh.id

private:
  Scene(FlimAPI &api) : api(api), camera(*this) {};
  friend class FlimAPI;
  friend class ::VulkanApplication;
};

} // namespace Flim
