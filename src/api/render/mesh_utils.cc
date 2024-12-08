#include "mesh_utils.hh"
#include "api/render/mesh.hh"
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

namespace Flim {

Mesh MeshUtils::createCube(vec3 size) {
  Mesh model;

  Vertex vertex;

  // add vertices
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        vertex.pos =
            vec3((j - 0.5) * size.x, (i - 0.5) * size.y, (k - 0.5) * size.z);
        vertex.uv = vec3(i, k, 0);
        model.vertices.push_back(vertex);
      }
    }
  }

  // add triangles
  model.triangles.push_back(uvec3(0, 1, 2));
  model.triangles.push_back(uvec3(1, 2, 3));
  // TODO

  return model;
}

Mesh MeshUtils::createSphere(float radius, int n_slices, int n_stacks) {
  Mesh model;

  Vertex vertex;

  // add top vertex
  vertex.pos = radius * vec3(0, 1, 0);
  int v0 = model.vertices.size();
  model.vertices.push_back(vertex);

  // generate vertices per stack / slice
  for (int i = 0; i < n_stacks - 1; i++) {
    auto phi = M_PI * double(i + 1) / double(n_stacks);
    for (int j = 0; j < n_slices; j++) {
      auto theta = 2.0 * M_PI * double(j) / double(n_slices);
      auto x = std::sin(phi) * std::cos(theta);
      auto y = std::cos(phi);
      auto z = std::sin(phi) * std::sin(theta);
      vertex.pos = radius * vec3(x, y, z);
      model.vertices.push_back(vertex);
    }
  }

  // add bottom vertex
  vertex.pos = radius * vec3(0, -1, 0);
  int v1 = model.vertices.size();
  model.vertices.push_back(vertex);

  // add top / bottom triangles
  for (int i = 0; i < n_slices; ++i) {
    auto i0 = i + 1;
    auto i1 = (i + 1) % n_slices + 1;
    model.triangles.push_back(uvec3(v0, i1, i0));
    i0 = i + n_slices * (n_stacks - 2) + 1;
    i1 = (i + 1) % n_slices + n_slices * (n_stacks - 2) + 1;
    model.triangles.push_back(uvec3(v1, i0, i1));
  }

  // add quads per stack / slice
  for (int j = 0; j < n_stacks - 2; j++) {
    auto j0 = j * n_slices + 1;
    auto j1 = (j + 1) * n_slices + 1;
    for (int i = 0; i < n_slices; i++) {
      auto i0 = j0 + i;
      auto i1 = j0 + (i + 1) % n_slices;
      auto i2 = j1 + (i + 1) % n_slices;
      auto i3 = j1 + i;
      model.triangles.push_back(uvec3(i0, i1, i2));
      model.triangles.push_back(uvec3(i0, i2, i3));
    }
  }
  return model;
}
}; // namespace Flim
