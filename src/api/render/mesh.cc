#include "mesh.hh"
#include "api/render/material.hh"
#include <glm/fwd.hpp>

namespace Flim {

const std::vector<Vertex> &Mesh::getVertices() const { return vertices; };
const Material &Mesh::getMaterial() const { return material; };
const std::vector<uint16> &Mesh::getTriangles() const { return indices; }

void Mesh::attachMaterial(Material m) { material = m; }

} // namespace Flim
