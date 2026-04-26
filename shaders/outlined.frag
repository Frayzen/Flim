#version 450

layout(location = 0) flat in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UniformMaterialObject {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  int materialMask;
} material;

layout(binding = 2) uniform OutlinedParam {
  uint outlined;
} outlinedParam;

// layout(binding = X) uniform sampler2D texSampler;

void main() {
    vec3 lightDirection = normalize(vec3(0, 0, -1));
    if (gl_PrimitiveID == outlinedParam.outlined)
      outColor = vec4(1.0, 0.0, 0.0, 1.0);
    else
      outColor = vec4(material.ambient + max(0, dot(lightDirection, normalize(fragNormal))) * material.diffuse, 1.0);
}
