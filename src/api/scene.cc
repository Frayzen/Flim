#include "scene.hh"
#include "api/tree/instance_object.hh"

namespace Flim {
void Scene::defaultRenderer(Renderer *renderer) { this->renderer = renderer; }

TreeObject Scene::instantiate(Mesh &mesh) {
  InstanceObject obj(&root, *this, mesh);
  return obj;
}
}; // namespace Flim
