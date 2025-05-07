#include "mesh_utils.hh"
#include "api/render/mesh.hh"
#include <Eigen/Eigen>
#include <Eigen/src/Core/Matrix.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>
#include <stdexcept>
#include <string>

namespace Flim {

Mesh MeshUtils::createCube(float side_length) {
  Mesh model;

  Vertex vertex{};

  float half = side_length / 2.0f;

  // Define the 8 vertices of the cube
  std::vector<Vector3f> positions = {
      {half, -half, -half},  {half, -half, half},  {-half, -half, half},
      {-half, -half, -half}, {half, half, -half},  {half, half, half},
      {-half, half, half},   {-half, half, -half},
  };

  // Add vertices to the mesh
  for (const auto &pos : positions) {
    vertex.pos = pos;
    vertex.normal = pos.normalized();
    model.vertices.push_back(vertex);
  }

  // Define the 12 triangles (2 per face)
  const std::vector<uint16_t> indices = {1, 2, 3, 7, 6, 5, 4, 5, 1, 5, 6, 2,
                                         2, 6, 7, 0, 3, 7, 0, 1, 3, 4, 7, 5,
                                         0, 4, 1, 1, 5, 2, 3, 2, 7, 4, 0, 7};
  // Add indices to the mesh
  for (const auto &tri : indices) {
    model.indices.push_back(tri);
  }

  return model;
}

Mesh MeshUtils::createSphere(float radius, int n_slices, int n_stacks) {
  Mesh model;

  Vertex vertex{};

  // add top vertex
  vertex.pos = radius * Vector3f(0, 1, 0);
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
      vertex.pos = radius * Vector3f(x, y, z);
      model.vertices.push_back(vertex);
    }
  }

  // add bottom vertex
  vertex.pos = radius * Vector3f(0, -1, 0);
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

Mesh MeshUtils::createNodalMesh() {
  Mesh model;
  model.vertices.push_back({});
  model.indices.push_back(0);
  model.indices.push_back(0);
  model.indices.push_back(0);
  return model;
}

static Transform getMeshTransformFromScene(const aiScene *scene) {
  int upAxis = 0;
  int upAxisSign = 1;
  int frontAxis = 0;
  int frontAxisSign = 1;
  int coordAxis = 0;
  int coordAxisSign = 1;
  scene->mMetaData->Get<int>("UpAxis", upAxis);
  scene->mMetaData->Get<int>("UpAxisSign", upAxisSign);
  scene->mMetaData->Get<int>("FrontAxis", upAxis);
  scene->mMetaData->Get<int>("FrontAxisSign", upAxisSign);
  scene->mMetaData->Get<int>("CoordAxis", upAxis);
  scene->mMetaData->Get<int>("CoordAxisSign", upAxisSign);
  aiVector3D upVec = upAxis == 0   ? aiVector3D(upAxisSign, 0, 0)
                     : upAxis == 1 ? aiVector3D(0, upAxisSign, 0)
                                   : aiVector3D(0, 0, upAxisSign);
  aiVector3D forwardVec = frontAxis == 0 ? aiVector3D(frontAxisSign, 0, 0)

                          : frontAxis == 1 ? aiVector3D(0, frontAxisSign, 0)

                                           : aiVector3D(0, 0, frontAxisSign);

  aiVector3D rightVec = coordAxis == 0   ? aiVector3D(coordAxisSign, 0, 0)
                        : coordAxis == 1 ? aiVector3D(0, coordAxisSign, 0)
                                         : aiVector3D(0, 0, coordAxisSign);

  aiMatrix4x4 mat(rightVec.x, rightVec.y, rightVec.z, 0.0f, upVec.x, upVec.y,

                  upVec.z, 0.0f, forwardVec.x, forwardVec.y, forwardVec.z, 0.0f,
                  0.0f, 0.0f, 0.0f, 1.0f);
  Transform t;
  Matrix3f mat3 = (*((Matrix4f *)&mat)).block<3, 3>(0, 0);
  t.rotation = Quaternionf(mat3);
  return t;
}

Mesh MeshUtils::loadFromFile(const char *path, bool smoothNormals) {
  std::cout << "Importing " << path << "..." << '\n';
  auto postProcessEffects = aiProcess_Triangulate |
                            aiProcess_JoinIdenticalVertices |
                            aiProcess_GenUVCoords | aiProcess_FlipUVs |
                            aiProcess_RemoveRedundantMaterials;
  if (smoothNormals)
    postProcessEffects |= aiProcess_GenSmoothNormals;
  else
    postProcessEffects |= aiProcess_GenNormals;
  static Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(path, postProcessEffects);

  if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      scene->mRootNode == nullptr)
    throw std::runtime_error("Could not load path: " + std::string(path));

  Matrix4f rot = *((Matrix4f *)&scene->mRootNode->mTransformation);

  for (uint i = 0; i < scene->mNumMeshes; i++) {
    Mesh m;
    std::cout << " = [MESH " << i << "] Creating mesh" << std::endl;
    std::cout << " | Retrieving vertices" << std::endl;
    auto mesh = scene->mMeshes[i];
    Vertex vtx{};
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
      aiVector3D v = mesh->mVertices[i];
      aiVector3D n = mesh->mNormals[i];
      vtx.pos = Vector3f(v.x, v.y, v.z);
      vtx.normal = Vector3f(n.x, n.y, n.z);
      vtx.uv = Vector2f(0, 0);
      m.vertices.push_back(vtx);
    }

    std::cout << " | Retrieving indices" << std::endl;
    // Retrieve indices (assuming triangles)
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
      aiFace face = mesh->mFaces[i];
      if (face.mNumIndices == 3) // Assuming triangles
      {
        m.indices.push_back(face.mIndices[0]);
        m.indices.push_back(face.mIndices[1]);
        m.indices.push_back(face.mIndices[2]);
      }
    }
    std::cout << " | Attaching material" << std::endl;
    m.attachMaterial(
        Material::createFrom(scene->mMaterials[mesh->mMaterialIndex]));
    /* m.transform = getMeshTransformFromScene(scene); */

    std::cout << " = [MESH " << i << "] Finished creating" << std::endl;
    return m;
  }
  return Mesh();
}

Mesh MeshUtils::createGrid(float length, int nbpts_width, int nbpts_height) {
  Mesh model;
  Vertex v{};
  v.normal = Vector3f(0, 0, 1);
  for (int i = 0; i < nbpts_height; i++) {
    for (int j = 0; j < nbpts_width; j++) {
      v.pos = Vector3f(j * length, i * length, 0);
      model.vertices.push_back(v);
    }
  }

  for (int i = 0; i < nbpts_width - 1; i++) {
    for (int j = 0; j < nbpts_height - 1; j++) {
      int bot_left = i + j * (nbpts_width);
      int bot_right = bot_left + 1;
      int top_left = i + (j + 1) * (nbpts_width);
      int top_right = top_left + 1;

      model.indices.push_back(top_left);
      model.indices.push_back(top_right);
      model.indices.push_back(bot_left);

      model.indices.push_back(top_right);
      model.indices.push_back(bot_right);
      model.indices.push_back(bot_left);
    }
  }
  return model;
}

}; // namespace Flim
