#pragma once

#include <assimp/material.h>
#include <fwd.hh>
#include <string>

namespace Flim {
class Material {
public:
  static Material createFrom(aiMaterial *mat);
  std::string name;
  Material() : Material("Default material") {};

  Vector3f diffuse;
  Vector3f ambient;
  Vector3f specular;
  float shininess;
  float shininess_strength;
  std::string texturePath;

private:
  Material(const char *name)
      : diffuse(0,0,0), ambient(0.5,0.5,0.5), specular(0,0,0), shininess(0),
        shininess_strength(0), texturePath(""), name(name) {};
  friend class MeshUtils;
};
}; // namespace Flim
