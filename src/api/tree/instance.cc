#include "instance.hh"
#include "api/scene.hh"
namespace Flim {

InstanceObject::InstanceObject(Scene &scene, Mesh &mesh)
    : scene(scene), mesh(mesh) {};

Renderer &InstanceObject::getRenderer() { return *scene.renderers[mesh.id]; }

} // namespace Flim
