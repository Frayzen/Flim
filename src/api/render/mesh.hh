#pragma once

#include "api/render/material.hh"
#include "api/transform.hh"
#include "fwd.h"
#include <glm/fwd.hpp>
#include <vector>

namespace Flim {

struct Vertex {
  vec3 pos;
  vec3 normal;
  vec2 uv;
};

class Mesh {

public:
  Transform transform;
  const Material &getMaterial() const;
  const std::vector<Vertex> &getVertices() const;
  const std::vector<uint16> &getTriangles() const;
  void attachMaterial(Material m);

private:
  Mesh() = default;

  Material material;
  std::vector<Vertex> vertices;
  std::vector<uint16> indices;

  friend class MeshUtils;
};
}; // namespace Flim
