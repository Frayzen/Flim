#pragma once

#include "api/render/material.hh"
#include "api/transform.hh"
#include <fwd.hh>
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
  ~Mesh();

private:
  Mesh() : bufferCreated(false) {};

  Material material;
  std::vector<Vertex> vertices;
  std::vector<uint16> indices;

  void updateBuffers();

protected:
  bool bufferCreated;
  Buffer indexBuffer;
  Buffer vertexBuffer;
  void cleanup();

  friend class MeshUtils;
};
}; // namespace Flim
