#include "mesh.hh"
#include "api/render/material.hh"
#include "api/tree/instance.hh"
#include <glm/fwd.hpp>

namespace Flim {

const std::vector<Vertex> &Mesh::getVertices() const { return vertices; };
const Material &Mesh::getMaterial() const { return material; };
const std::vector<uint16> &Mesh::getTriangles() const { return indices; }

Mesh::Mesh() : id(curid++) {};
void Mesh::attachMaterial(Material m) { material = m; }
void Mesh::updateModelViews() {
  for (auto &istc : instances) {
    modelViews[istc.getId()] = istc.transform.getViewMatrix();
  }
}

} // namespace Flim
