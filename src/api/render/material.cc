#include "material.hh"
#include <fwd.hh>

namespace Flim {
Material Material::createFrom(aiMaterial *aiMat) {
  Material m(aiMat->GetName().C_Str());

  aiColor3D amb(0.15f);
  aiColor3D dif(0.7f);
  aiColor3D spec(0.1f);

  /* float shine = 32.0; */
  aiMat->Get(AI_MATKEY_COLOR_AMBIENT, amb);
  aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, dif);
  aiMat->Get(AI_MATKEY_COLOR_SPECULAR, spec);

  m.ambient = Vector3f(amb.r, amb.g, amb.b);
  m.diffuse = Vector3f(dif.r, dif.g, dif.b);
  m.specular = Vector3f(spec.r, spec.g, spec.b);

  // FOR LATER HANDLE TEXTS
  /* for (unsigned int i = 0; i < aiMat->GetTextureCount(aiTextureType_DIFFUSE); */
  /*      i++) { */
  /*   aiString textureName; */
  /*   aiMat->GetTexture(aiTextureType_DIFFUSE, i, &textureName); */
  /*   m.texturePath = textureName.C_Str(); */
  /*   std::cout << m.texturePath << std::endl; */
  /* } */
  /* aiMat->Get(AI_MATKEY_SHININESS, shine); */
  return m;
}
}; // namespace Flim
