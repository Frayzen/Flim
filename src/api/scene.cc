#include "scene.hh"
#include "api/parameters/compute_params.hh"
#include "api/tree/instance.hh"
#include "vulkan/rendering/renderer.hh"
#include <Eigen/src/Core/Matrix.h>
#include <cassert>

namespace Flim {

Instance &Scene::instantiate(Mesh &mesh) const {
  assert(renderers.contains(mesh.id) &&
         "Please register the mesh before instatiating it");
  Instance &obj = mesh.instances.emplace_back(*this, mesh);
  obj.id = mesh.instances.size() - 1;
  return obj;
}

void Scene::registerMesh(Mesh &mesh, RenderParams &params) {
  assert(params.usable());
  renderers.insert(
      std::pair(mesh.id, std::make_shared<Renderer>(mesh, params)));
}

void Scene::registerComputer(ComputeParams &cparams, int dispatchX,
                             int dispatchY, int dispatchZ) {
  assert(cparams.usable());
  computers.emplace_back(std::make_shared<Computer>(
      Vector3i(dispatchX, dispatchY, dispatchZ), cparams));
}

}; // namespace Flim
