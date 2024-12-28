#include "scene.hh"
#include "api/tree/instance.hh"
#include "vulkan/rendering/renderer.hh"
#include <cassert>

namespace Flim {

InstanceObject &Scene::instantiate(Mesh &mesh) {
  assert(renderers.contains(mesh.id) &&
         "Please register the mesh before instatiating it");
  return mesh.instances.emplace_back(*this, mesh);
}

void Scene::registerMesh(Mesh &mesh, RenderParams &params) {
  renderers.insert(
      std::pair(mesh.id, std::make_shared<Renderer>(mesh, params)));
}

}; // namespace Flim
