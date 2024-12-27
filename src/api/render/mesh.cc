#include "mesh.hh"
#include "api/render/material.hh"
#include "api/tree/instance_object.hh"
#include <glm/fwd.hpp>

namespace Flim {

const std::vector<Vertex> &Mesh::getVertices() const { return vertices; };
const Material &Mesh::getMaterial() const { return material; };
const std::vector<uint16> &Mesh::getTriangles() const { return indices; }

void Mesh::registerInstance(InstanceObject& instance)
{
  assert(&instance.mesh == this);
  instances.push_back(&instance);
}
void Mesh::attachMaterial(Material m) { material = m; }

} // namespace Flim
