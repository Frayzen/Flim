#include "mesh.hh"
#include "api/render/material.hh"
#include "vulkan/buffers/buffer_utils.hh"
#include <glm/fwd.hpp>

namespace Flim {

const std::vector<Vertex> &Mesh::getVertices() const { return vertices; };
const Material &Mesh::getMaterial() const { return material; };
const std::vector<uint16> &Mesh::getTriangles() const { return indices; }

void Mesh::attachMaterial(Material m) { material = m; }

void Mesh::updateBuffers() {
  if (bufferCreated)
    cleanup();
  assert(vertices.size() > 0);
  assert(indices.size() > 0);
  populateBufferFromData(vertexBuffer, vertices.data(),
                         vertices.size() * sizeof(vertices[0]));
  populateBufferFromData(indexBuffer, indices.data(),
                         indices.size() * sizeof(indices[0]));
  bufferCreated = true;
}

Mesh::~Mesh() { cleanup(); }

void Mesh::cleanup() {
  destroyBuffer(indexBuffer);
  destroyBuffer(vertexBuffer);
}

} // namespace Flim
