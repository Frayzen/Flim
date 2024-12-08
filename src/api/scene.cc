#include "scene.hh"
#include "api/tree/instance_object.hh"

namespace Flim {
void Scene::defaultRenderer(Renderer *renderer) { this->renderer = renderer; }

InstanceObject Scene::instantiate(Mesh &mesh) {
  InstanceObject obj = InstanceObject::instantiate(mesh, &root);
  return obj;
}

const RootObject &Scene::getRoot() { return root; }

}; // namespace Flim
