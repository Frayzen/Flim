#pragma once

#include "fwd.h"
#include <assimp/material.h>
#include <string>

namespace Flim {
class Material {
public:
  static Material createFrom(aiMaterial *mat);
  std::string name;
  Material() : name("undefined") {};

  vec3 diffuse;
  vec3 ambient;
  vec3 specular;
  float shininess;
  float shininess_strength;
  std::string texturePath;

private:
  Material(const char *name) : name(name) {};
};
}; // namespace Flim
