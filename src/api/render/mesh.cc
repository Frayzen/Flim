#include "mesh.hh"
#include "api/render/material.hh"
#include "api/tree/instance.hh"

namespace Flim {

const std::vector<Vertex> &Mesh::getVertices() const { return vertices; };
const Material &Mesh::getMaterial() const { return material; };
const std::vector<uint16_t> &Mesh::getTriangles() const { return indices; }

Mesh::Mesh() : id(meshid++), material(), vertices(), indices() {};
void Mesh::attachMaterial(Material m) { material = m; }

} // namespace Flim
