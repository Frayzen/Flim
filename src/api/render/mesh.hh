#pragma once

#include "api/tree/instance.hh"
#include "api/render/material.hh"
#include "api/transform.hh"
#include <fwd.hh>
#include <glm/fwd.hpp>
#include <span>
#include <vector>

class CommandPoolManager;
class Renderer;

namespace Flim {

class Instance;

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
  std::vector<Instance> instances;
  const Material &getMaterial() const;
  const std::vector<Vertex> &getVertices() const;
  const std::vector<uint16> &getTriangles() const;
  void attachMaterial(Material m);
  void updateModelViews();

  std::vector<Vertex> vertices;
  std::vector<uint16> indices;

  std::span<glm::mat4> modelViews;

protected:
  Mesh() : id(curid++), material() {};

  Material material;

  friend class Scene;
  friend class MeshUtils;
  friend class ::CommandPoolManager;
  friend class ::Renderer;
};

}; // namespace Flim
