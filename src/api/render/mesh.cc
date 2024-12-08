#include "mesh.hh"

namespace Flim {

const std::vector<Vertex> &Mesh::getVertices() const { return vertices; };
const std::vector<uvec3> &Mesh::getTriangles() const { return indices; }
} // namespace Flim
