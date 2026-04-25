#include "mesh.hh"
#include "api/render/material.hh"
#include "api/tree/instance.hh"

namespace Flim {

const std::vector<Vertex> &Mesh::getVertices() const { return vertices; };
const Material &Mesh::getMaterial() const { return material; };
const std::vector<Triangle> &Mesh::getTriangles() const { return triangles; }

Mesh::Mesh() : id(meshid++), material(), vertices(), triangles() {};
void Mesh::attachMaterial(Material m) { material = m; }

} // namespace Flim
