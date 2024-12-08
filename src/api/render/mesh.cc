#include "mesh.hh"
#include <glm/fwd.hpp>

namespace Flim {

const std::vector<Vertex> &Mesh::getVertices() const { return vertices; };
const std::vector<uint16> &Mesh::getTriangles() const { return indices; }
} // namespace Flim
