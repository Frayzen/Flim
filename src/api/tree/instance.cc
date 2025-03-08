#include "instance.hh"
#include "api/scene.hh"
#include "vulkan/rendering/renderer.hh"
namespace Flim {

Instance::Instance(const Scene &scene, Mesh &mesh) : scene(scene), mesh(mesh) {};

Renderer &Instance::getRenderer() {
  auto res = scene.renderers.find(mesh.id);
  assert(res != scene.renderers.end());
  return *res->second;
}

int Instance::getId() const { return id; };

} // namespace Flim
