#include "scene.hh"
#include "api/parameters/compute_params.hh"
#include "api/tree/instance.hh"
#include "vulkan/rendering/renderer.hh"
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

void Scene::registerMesh(Mesh &mesh, RenderParams &rparams,
                         ComputeParams &cparams) {
  assert(cparams.usable());
  assert(rparams.usable());
  renderers.insert(
      std::pair(mesh.id, std::make_shared<Renderer>(mesh, rparams)));
  computers.insert(
      std::pair(mesh.id, std::make_shared<Computer>(cparams)));
}

}; // namespace Flim
