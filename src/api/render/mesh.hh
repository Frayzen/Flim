#pragma once

#include "fwd.h"
#include <glm/fwd.hpp>
#include <vector>

namespace Flim {

struct Vertex {
  vec3 pos;
  vec2 uv;
};

class Mesh {

public:
  const std::vector<Vertex>& getVertices() const;
  const std::vector<uvec3>& getTriangles() const;

private:
  Mesh() = default;

  std::vector<Vertex> vertices;
  std::vector<uvec3> indices;

  friend class MeshUtils;
};
}; // namespace Flim
