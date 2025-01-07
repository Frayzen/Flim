#pragma once

#include "api/fwd.hh"
#include "api/parameters/compute_params.hh"
#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/tree/camera.hh"
#include "vulkan/computing/computer.hh"
#include "vulkan/rendering/renderer.hh"

class VulkanApplication;

namespace Flim {

class Instance;

class Scene {

public:
  Instance &instantiate(Mesh &mesh) const;

  void registerMesh(Mesh &mesh, RenderParams &params);
  void registerComputer(ComputeParams &cparams);

  FlimAPI &api;
  Camera camera;
  std::map<int, std::shared_ptr<Renderer>> renderers;
  std::vector<std::shared_ptr<Computer>> computers;

private:
  Scene(FlimAPI &api) : api(api), camera(*this) {};
  friend class FlimAPI;
  friend class ::VulkanApplication;
};

} // namespace Flim
