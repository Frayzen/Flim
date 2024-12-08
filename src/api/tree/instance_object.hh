#pragma once

#include "api/render/mesh.hh"
#include "api/tree/tree_object.hh"
#include <memory>
namespace Flim {
class InstanceObject : public TreeObject {
public:
  InstanceObject(TreeObject *parent, Mesh &mesh)
      : TreeObject(parent), mesh(mesh) {};
  static InstanceObject &instantiate(Mesh &mesh, TreeObject *parent) {
    parent->children.push_back(std::make_shared<InstanceObject>(parent, mesh));
    return *std::dynamic_pointer_cast<InstanceObject>(parent->children.back());
  }

  const Mesh &mesh;
};
} // namespace Flim
