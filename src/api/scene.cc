#include "scene.hh"
#include "api/tree/instance_object.hh"

namespace Flim {
void Scene::defaultRenderer(Renderer *renderer) { this->renderer = renderer; }

InstanceObject &Scene::instantiate(Mesh &mesh) {
  return InstanceObject::instantiate(mesh, &root);
}

const RootObject &Scene::getRoot() { return root; }

}; // namespace Flim
