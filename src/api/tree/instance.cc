#include "instance.hh"
#include "api/scene.hh"
namespace Flim {

Instance::Instance(Scene &scene, Mesh &mesh) : scene(scene), mesh(mesh) {};

Renderer &Instance::getRenderer() { return *scene.renderers[mesh.id]; }

int Instance::getId() const { return id; };

} // namespace Flim
