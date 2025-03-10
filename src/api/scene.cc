#include "scene.hh"
#include "api/parameters/compute_params.hh"
#include "api/tree/instance.hh"
#include "utils/checks.hh"
#include "vulkan/rendering/renderer.hh"
#include "api/flim_api.hh"
#include <Eigen/src/Core/Matrix.h>
#include <cassert>
#include <stdexcept>

namespace Flim {

Instance &Scene::instantiate(Mesh &mesh) const {
  CHECK(!api.graphicsLoaded(), "You cannot instantiate a mesh after having loaded the graphics");
  CHECK(renderers.contains(mesh.id),
        "Please register the mesh before instatiating it");
  Instance &obj = mesh.instances.emplace_back(*this, mesh);
  obj.id = mesh.instances.size() - 1;
  return obj;
}

const Renderer &Scene::registerMesh(Mesh &mesh, RenderParams &params) {
  CHECK(!api.graphicsLoaded(), "You cannot register a mesh after having loaded the graphics");
  if (renderers.contains(mesh.id))
    throw std::runtime_error("You cannot register twice the same mesh");
  assert(params.usable());
  assert(
      renderers
          .insert(std::pair(mesh.id, std::make_shared<Renderer>(mesh, params)))
          .second);
  return *renderers[mesh.id];
}

void Scene::registerComputer(ComputeParams &cparams, int dispatchX,
                             int dispatchY, int dispatchZ) {
  CHECK(!api.graphicsLoaded(), "You cannot register a compute shader after having loaded the graphics");
  assert(cparams.usable());
  computers.emplace_back(std::make_shared<Computer>(
      Vector3i(dispatchX, dispatchY, dispatchZ), cparams));
}

}; // namespace Flim
