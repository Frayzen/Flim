#pragma once
#include "api/render/mesh.hh"
#include <fwd.hh>
#include <glm/fwd.hpp>
#include <string>

namespace Flim {

class MeshUtils {
public:
  static Mesh createCube(float side_length = 1);
  static Mesh createSphere(float radius = 1.0f, int n_slices = 10, int n_stacks = 10);
  static Mesh loadFromFile(const char* path);
};

} // namespace Flim
