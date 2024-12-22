#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UniformMaterialObject {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
} material;

const vec4 colors[] = {
  vec4(0,0,1,1),
  vec4(0,1,0,1),
  vec4(0,1,1,1),
  vec4(1,0,0,1),
  vec4(1,0,1,1),
  vec4(1,1,0,1),
  vec4(1,1,1,1),
};


layout(binding = 2) uniform sampler2D texSampler;


void main() {
    // outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0);
    // outColor = vec4(fragColor.rgb * 0.1f, 1.0);

    vec3 lightDirection = normalize(vec3(1, -2, 1));

    outColor = vec4(material.ambient + max(0, dot(-lightDirection, normalize(fragNormal))) * material.diffuse, 1.0);

    // int col = gl_PrimitiveID;
    // outColor = colors[col * 3 % 8];
}
