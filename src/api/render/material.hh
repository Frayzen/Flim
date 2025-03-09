#pragma once

#include <assimp/material.h>
#include <fwd.hh>
#include <string>

// Ambient is bits 1-2
#define MASK_AMBIENT (0b111 << 0)
#define MASK_AMBIENT_UNUSED (0 << 0)
#define MASK_AMBIENT_PER_VERTEX (1 << 0)
#define MASK_AMBIENT_PER_TRI (2 << 0)
#define MASK_AMBIENT_TEXT (3 << 0) // for later

// Diffuse is bits 3-4
#define MASK_DIFFUSE (0b111 << 2)
#define MASK_DIFFUSE_UNUSED (0 << 2)
#define MASK_DIFFUSE_PER_VERTEX (1 << 2)
#define MASK_DIFFUSE_PER_TRI (2 << 2)
#define MASK_DIFFUSE_TEXT (3 << 2) // for later

// Specular is bits 5-6
#define MASK_SPECULAR (0b111 << 4)
#define MASK_SPECULAR_UNUSED (0 << 4)
#define MASK_SPECULAR_PER_VERTEX (1 << 4)
#define MASK_SPECULAR_PER_TRI (2 << 4)
#define MASK_SPECULAR_TEXT (3 << 4) // for later

namespace Flim {
class Material {
public:
  static Material createFrom(aiMaterial *mat);
  std::string name; // for debug purposes
  Material() : Material("Default material") {};

  Vector3f diffuse;
  Vector3f ambient;
  Vector3f specular;

  int mask;

private:
  Material(const char *name)
      : diffuse(0, 0, 0), ambient(0.5, 0.5, 0.5), specular(0, 0, 0), name(name),
        mask(0) {};
  friend class MeshUtils;
};
}; // namespace Flim
