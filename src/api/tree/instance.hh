#pragma once

#include "api/scene.hh"
#include "api/transform.hh"
#include "vulkan/rendering/renderer.hh"
namespace Flim {

class Mesh;

class InstanceObject {
public:
  Transform transform;
  Scene &scene;
  InstanceObject(Scene &scene, Mesh &mesh);

  static InstanceObject &instantiate(Mesh &mesh, TreeObject *parent);
  Renderer &getRenderer();
  const Mesh &mesh;
};
} // namespace Flim
