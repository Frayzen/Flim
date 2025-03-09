#version 450

layout(binding = 0) uniform UniformLocationObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ulo;

layout(binding = 2) uniform PointDesc {
  float size;
  float edgeSize;
  bool applyDiffuse;
} pointDesc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 3) in mat4 inInstanceMat;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec2 fragPos;

void main() {
    gl_Position = ulo.proj * ulo.view * ulo.model * inInstanceMat * vec4(inPosition, 1.0);
    vec4 pos = gl_Position;
    if (pos.w != 0)
      pos /= pos.w;
    fragColor = inPosition;
    fragNormal = inNormal;
    fragTexCoord = inTexCoord;
    float depth = pos.z;
    fragPos = pos.xy;
    gl_PointSize = (pointDesc.size + pointDesc.edgeSize) / abs(depth);
}
