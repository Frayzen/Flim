#pragma once
#include "api/render/mesh.hh"
#include <fwd.hh>
#include <string>

namespace Flim {

class MeshUtils {
public:
  static Mesh createCube(float side_length = 1.0f);
  static Mesh createSphere(float radius = 1.0f, int n_slices = 10, int n_stacks = 10);
  static Mesh createNodalMesh();
  static Mesh loadFromFile(std::string path);
};

} // namespace Flim
