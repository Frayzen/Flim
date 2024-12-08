#include "mesh_utils.hh"
#include "api/render/mesh.hh"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstddef>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <stdexcept>

namespace Flim {

Mesh MeshUtils::createCube(float side_length) {
  Mesh model;

  Vertex vertex;

  float half = side_length / 2.0f;

  // Define the 8 vertices of the cube
  std::vector<glm::vec3> positions = {
      {-half, -half, half}, {half, -half, half},   {-half, half, half},
      {half, half, half},   {-half, -half, -half}, {half, -half, -half},
      {-half, half, -half}, {half, half, -half},
  };

  // Add vertices to the mesh
  for (const auto &pos : positions) {
    vertex.pos = pos;
    model.vertices.push_back(vertex);
  }

  // Define the 12 triangles (2 per face)
  std::vector<uint16> indices = {// Top
                                 2, 6, 7, 2, 3, 7,

                                 // Bottom
                                 0, 4, 5, 0, 1, 5,

                                 // Left
                                 0, 2, 6, 0, 4, 6,

                                 // Right
                                 1, 3, 7, 1, 5, 7,

                                 // Front
                                 0, 2, 3, 0, 1, 3,

                                 // Back
                                 4, 6, 7, 4, 5, 7};

  // Add indices to the mesh
  for (const auto &tri : indices) {
    model.indices.push_back(tri);
  }

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
    model.indices.push_back(v0);
    model.indices.push_back(i1);
    model.indices.push_back(i0);
    i0 = i + n_slices * (n_stacks - 2) + 1;
    i1 = (i + 1) % n_slices + n_slices * (n_stacks - 2) + 1;
    model.indices.push_back(v1);
    model.indices.push_back(i0);
    model.indices.push_back(i1);
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

      model.indices.push_back(i0);
      model.indices.push_back(i1);
      model.indices.push_back(i2);
      model.indices.push_back(i0);
      model.indices.push_back(i2);
      model.indices.push_back(i3);
    }
  }
  return model;
}

Mesh MeshUtils::loadFromFile(std::string path) {
  std::cout << "Importing " << path << "..." << '\n';
  static Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(
      path.c_str(),
      aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
          aiProcess_GenUVCoords | aiProcess_FlipUVs |
          aiProcess_RemoveRedundantMaterials |
          aiProcess_GenSmoothNormals /* or aiProcess_GenNormals */);

  std::cout << scene << std::endl;
  if (scene == NULL || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      scene->mRootNode == NULL)
    std::runtime_error("Could not load " + path);

  Mesh m;
  std::cout << " = Creating the mesh..." << '\n';

  for (uint i = 0; i < scene->mNumMeshes; i++) {
    auto mesh = scene->mMeshes[i];
    Vertex vtx{};
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
      auto v = mesh->mVertices[i];
      vtx.pos = {v.x, v.y, v.z};
      m.vertices.emplace_back(vtx);
    }
    // Retrieve indices (assuming triangles)
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
      aiFace face = mesh->mFaces[i];
      if (face.mNumIndices == 3) // Assuming triangles
      {
        m.indices.emplace_back(face.mIndices[0]);
        m.indices.emplace_back(face.mIndices[1]);
        m.indices.emplace_back(face.mIndices[2]);
      }
    }
  }
  return m;
}

}; // namespace Flim
