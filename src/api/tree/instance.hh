#pragma once

#include "api/transform.hh"
#include "vulkan/rendering/renderer.hh"
namespace Flim {

class Mesh;
class Scene;

class Instance {
public:
  Transform transform;
  Instance(const Scene &scene, Mesh &mesh);

  Renderer &getRenderer();
  const Scene &scene;
  const Mesh &mesh;
  int getId() const;

private:
  int id;
  friend class Scene;
};
}; // namespace Flim
