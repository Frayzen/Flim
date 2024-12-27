#pragma once

#include "api/render/material.hh"
#include "api/transform.hh"
#include <fwd.hh>
#include <glm/fwd.hpp>
#include <vector>

class CommandPoolManager;
class Renderer;

namespace Flim {

struct Vertex {
  vec3 pos;
  vec3 normal;
  vec2 uv;
};

static int curid = 0;
class Mesh {

public:
  int id;
  Transform transform;
  const Material &getMaterial() const;
  const std::vector<Vertex> &getVertices() const;
  const std::vector<uint16> &getTriangles() const;
  void attachMaterial(Material m);

protected:
  Mesh() : id(curid++) {};

  Material material;
  std::vector<Vertex> vertices;
  std::vector<uint16> indices;

  friend class MeshUtils;
  friend class ::CommandPoolManager;
  friend class ::Renderer;
};

}; // namespace Flim
