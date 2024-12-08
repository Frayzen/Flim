#pragma once
#include "api/render/mesh.hh"
#include "api/scene.hh"
#include "fwd.h"
#include <glm/fwd.hpp>

namespace Flim {

class MeshUtils {
public:
  MeshUtils(Scene &scene) : scene(scene) {};
  Scene &scene;
  Mesh createCube(vec3 size);
  Mesh createSphere(float radius = 1.0f, int n_slices = 10, int n_stacks = 10);
};

} // namespace Flim
