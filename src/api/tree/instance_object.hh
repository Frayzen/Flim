#pragma once

#include "api/render/mesh.hh"
#include "api/scene.hh"
#include "api/tree/tree_object.hh"
namespace Flim {
class InstanceObject : public TreeObject {
public:
  InstanceObject(TreeObject *parent, Scene &scene, Mesh &mesh)
      : TreeObject(parent, scene), mesh(mesh) {};

private:
  Mesh &mesh;
};
} // namespace Flim
