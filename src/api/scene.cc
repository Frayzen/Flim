#include "scene.hh"
#include "api/tree/instance_object.hh"
#include "api/tree/tree_object.hh"
#include "vulkan/rendering/renderer.hh"
#include <cassert>

namespace Flim {

InstanceObject &Scene::instantiate(Mesh &mesh) {
  assert(renderers.contains(mesh.id) &&
         "Please register the mesh before instatiating it");
  return InstanceObject::instantiate(mesh, &root);
}

const RootObject &Scene::getRoot() { return root; }

void Scene::registerMesh(Mesh &mesh, RenderParams &params) {
  renderers.insert(
      std::pair(mesh.id, std::make_shared<Renderer>(mesh, params)));
}

}; // namespace Flim
