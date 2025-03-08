#pragma once

#include "api/render/material.hh"
#include "api/transform.hh"
#include "api/tree/instance.hh"
#include <cstdint>
#include <fwd.hh>
#include <span>
#include <vector>

class CommandPoolManager;
class Renderer;

namespace Flim {

class Instance;

struct Vertex {
  Vector3f pos;
  Vector3f normal;
  Vector2f uv;
};

static int meshid = 0;
class Mesh {

public:
  int id;
  Transform transform;
  std::vector<Instance> instances;
  const Material &getMaterial() const;
  const std::vector<Vertex> &getVertices() const;
  const std::vector<uint16_t> &getTriangles() const;
  void attachMaterial(Material m);

  std::vector<Vertex> vertices;
  std::vector<uint16_t> indices;

  std::span<Matrix4f> modelViews;

protected:
  Mesh();

  Material material;

  friend class Scene;
  friend class MeshUtils;
  friend class ::CommandPoolManager;
  friend class ::Renderer;
};

}; // namespace Flim
