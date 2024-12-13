#include "material.hh"

namespace Flim {
Material Material::createFrom(aiMaterial *aiMat) {
  Material m(aiMat->GetName().C_Str());

  aiColor3D amb(0.15f);
  aiColor3D dif(0.7f);
  aiColor3D spec(0.1f);

  /* float shine = 32.0; */
  /* aiMat->Get(AI_MATKEY_COLOR_AMBIENT, amb); */
  /* aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, dif); */
  /* aiMat->Get(AI_MATKEY_COLOR_SPECULAR, spec); */

  m.ambient = vec3(amb.r, amb.g, amb.b);
  m.diffuse = vec3(dif.r, dif.g, dif.b);
  m.specular = vec3(spec.r, spec.g, spec.b);

  /* aiMat->Get(AI_MATKEY_SHININESS, shine); */
  return m;
}
}; // namespace Flim
