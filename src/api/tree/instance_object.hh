#pragma once

#include "api/render/mesh.hh"
#include "api/tree/tree_object.hh"
namespace Flim {
class InstanceObject : public TreeObject {
public:
  InstanceObject(TreeObject *parent, Mesh &mesh)
      : TreeObject(parent), mesh(mesh) {};
  static InstanceObject &instantiate(Mesh &mesh, TreeObject *parent) {
    InstanceObject& instance = parent->append<InstanceObject>(mesh);
    instance.transform = mesh.transform;
    return instance;
  }

  const Mesh &mesh;
};
} // namespace Flim
