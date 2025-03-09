#version 450

layout(location = 0) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UniformMaterialObject {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  int materialMask;
} material;


// layout(binding = X) uniform sampler2D texSampler;

void main() {
    vec3 lightDirection = normalize(vec3(1, -2, 1));
    outColor = vec4(material.ambient + max(0, dot(-lightDirection, normalize(fragNormal))) * material.diffuse, 1.0);
}
