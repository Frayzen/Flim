#include "scene.hh"
#include "api/tree/instance.hh"
#include "vulkan/rendering/renderer.hh"
#include <cassert>

namespace Flim {

Instance &Scene::instantiate(Mesh &mesh) {
  assert(renderers.contains(mesh.id) &&
         "Please register the mesh before instatiating it");
  Instance& obj = mesh.instances.emplace_back(*this, mesh);
  obj.id = mesh.instances.size() - 1; 
  return obj;
}

void Scene::registerMesh(Mesh &mesh, RenderParams &params) {
  renderers.insert(
      std::pair(mesh.id, std::make_shared<Renderer>(mesh, params)));
}

}; // namespace Flim
