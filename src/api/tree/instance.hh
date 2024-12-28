#pragma once

#include "api/scene.hh"
#include "api/transform.hh"
#include "vulkan/rendering/renderer.hh"
namespace Flim {

class Mesh;

class Instance {
public:
  Transform transform;
  Scene &scene;
  Instance(Scene &scene, Mesh &mesh);

  static Instance &instantiate(Mesh &mesh, TreeObject *parent);

  Renderer &getRenderer();
  const Mesh &mesh;
  int getId() const;

private:
  int id;
  friend class Scene;
};
} // namespace Flim
