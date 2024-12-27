#pragma once

#include "api/render/mesh.hh"
#include "api/scene.hh"
#include "api/tree/tree_object.hh"
#include "vulkan/rendering/renderer.hh"
namespace Flim {

class InstanceObject : public TreeObject {
public:
  InstanceObject(TreeObject *parent, Mesh &mesh)
      : TreeObject(parent), mesh(mesh) {
    mesh.registerInstance(*this);
  };

  static InstanceObject &instantiate(Mesh &mesh, TreeObject *parent) {
    InstanceObject &instance = parent->append<InstanceObject>(mesh);
    instance.transform = mesh.transform;
    return instance;
  }

  Renderer &getRenderer() { return *scene.renderers[mesh.id]; }

  const Mesh &mesh;
};
} // namespace Flim
