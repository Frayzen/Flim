#include "scene.hh"
#include "api/tree/instance_object.hh"
#include "api/tree/tree_object.hh"

namespace Flim {

Renderer emptyRenderer{};

void Scene::invalidateRenderer() { invalidatedRenderer = true; }

void Scene::defaultRenderer(Renderer &renderer) {
  this->renderer = renderer;
}

InstanceObject &Scene::instantiate(Mesh &mesh) {
  return InstanceObject::instantiate(mesh, &root);
}

const RootObject &Scene::getRoot() { return root; }

}; // namespace Flim
