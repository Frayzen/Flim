#pragma once
#include "api/render/mesh.hh"
#include <fwd.hh>

namespace Flim {

class MeshUtils {
public:
  static Mesh createCube(float side_length = 1.0f);
  static Mesh createSphere(float radius = 1.0f, int n_slices = 10, int n_stacks = 10);
  static Mesh loadFromFile(const char* path);
  static Mesh createNodalMesh();
};

} // namespace Flim
